<?php
// $Id$

/**
 * @file
 * Provides wrappers allowing easy usage of the entity metadata.
 */

/**
 * A common base class for all wrappers.
 */
abstract class EntityMetadataWrapper {

  protected $type;
  protected $data;
  protected $info;
  protected $cache = array();

  /**
   * Construct a new wrapper object.
   *
   * @param $type
   *   The type of the passed data.
   * @param $data
   *   Optional. The data to wrap.
   * @param $info
   *   Optional. Used internally to pass info about properties down the tree.
   */
  public function __construct($type, &$data = NULL, $info = array()) {
    $this->type = $type;
    $this->data = &$data;
    $this->info = $info + array(
      'clear' => array(),
      'langcode' => NULL,
      'getter callback' => 'entity_metadata_verbatim_get',
    );
  }

  /**
   * Gets info about the wrapped data.
   *
   * @return Array
   *   Keys set are all keys as specified for a property in hook_entity_info()
   *   as well as possible the following keys:
   *    - name: If this wraps a property, the name of the property.
   *    - parent: The parent wrapper, if any.
   *    - langcode: The language code, if this data is language specific.
   */
  public function info() {
    return $this->info;
  }

  /**
   * Gets the (entity)type of the wrapped data.
   */
  public function type() {
    return $this->type;
  }

  /**
   * If the wrapped data is language specific, get the language object.
   *
   * @return
   *   The language object or NULL if the data is language neutral.
   */
  public function getLanguage() {
    if (isset($this->info['langcode']) && $list = language_list()) {
      return $list[$this->info['langcode']];
    }
    return NULL;
  }

  /**
   * Returns the wrapped data. If no options are given the data is returned as
   * described in the info
   *
   * @param $options
   *   (optional) A keyed array of options:
   *   - sanitize: A boolean flag indicating that textual properties should be
   *     sanitized for display to a web browser. Defaults to FALSE.
   *   - decode: If set to TRUE and some textual data is already sanitized, it
   *     strips HTML tags and decodes HTML entities. Defaults to FALSE.
   */
  public function value(array $options = array()) {
    if (!isset($this->data) && isset($this->info['name'])) {
      // Retrieve entity from the parent entity.
      $info = $this->info;
      if (function_exists($info['getter callback'])) {
        $options = array('language' => $this->getLanguage(), 'absolute' => TRUE);
        $this->data = $info['getter callback']($info['parent']->value(), $options, $info['name'], $info['parent']->type);
      }
      if (!isset($this->data)) {
        throw new EntityMetadataWrapperException('Entity property ' . check_plain($info['name']) . " isn't set.");
      }
    }
    elseif (!isset($this->data)) {
      throw new EntityMetadataWrapperException('There is no data set, so you cannot retrieve any values.');
    }
    return $this->applyOptions($this->data, $options);
  }

  /**
   * Returns the raw, unprocessed data. Most times this is the same as returned
   * by value(), however for already processed and sanitized textual data, this
   * will return the unprocessed data in contrast to value().
   */
  public function raw() {
    if (isset($this->info['name']) && isset($this->info['parent'])) {
      if (!empty($this->info['raw getter callback']) && function_exists($function = $this->info['raw getter callback'])) {
        $options = array('language' => $this->getLanguage(), 'absolute' => TRUE);
        return $function($this->info['parent']->value(), $options, $this->info['name'], $this->info['parent']->type);
      }
    }
    // Else return the usual value, which should be raw in this case.
    return $this->value();
  }

  /**
   * Returns whether data is available to work with. Also if we wrap a property,
   * it ensures the value of the property is loaded and so available.
   *
   * @return
   *   If we operate without any data FALSE, else TRUE.
   */
  protected function dataAvailable() {
    if (!isset($this->data) && isset($this->info['name']) && $this->info['parent']->dataAvailable()) {
      // Make sure it is loaded.
      $this->value();
      return TRUE;
    }
    return isset($this->data);
  }

  protected function applyOptions(&$data, array $options) {
    return $data;
  }

  /**
   * Set a new data value.
   */
  public function set($value) {
    if (empty($this->info['parent'])) {
      $this->data = $value;
    }
    elseif (!empty($this->info['setter callback']) && function_exists($this->info['setter callback'])) {
      $this->info['setter callback']($this->info['parent']->value(), $this->info['name'], $value, $this->info['langcode'], $this->info['parent']->type);
      $this->cache = array();
      // Clear the cache of properties dependent on this value.
      foreach ($this->info['clear'] as $name) {
        unset($this->info['parent']->cache[$name]);
      }
      unset($this->data);
    }
    else {
      throw new EntityMetadataWrapperException('Entity property ' . check_plain($this->info['name']) . " doesn't support writing.");
    }
    return $this;
  }

  public function __toString() {
    return isset($this->info) ? 'Property ' . $this->info['name'] : $this->type;
  }
}

/**
 * Wraps a single value.
 */
class EntityValueWrapper extends EntityMetadataWrapper {

  /**
   * Overrides EntityMetadataWrapper#applyOptions().
   * Sanitizes or decodes textual data if necessary.
   */
  protected function applyOptions($data, array $options) {
    if ($this->type == 'text') {
      $info = $this->info + array('sanitized' => FALSE, 'sanitize' => 'check_plain');
      $options += array('sanitize' => FALSE, 'decode' => FALSE);
      if ($options['sanitize'] && !$info['sanitized']) {
        return $info['sanitize']($data);
      }
      elseif ($options['decode'] && $info['sanitized']) {
        return decode_entities(strip_tags($data));
      }
    }
    return $data;
  }
}

/**
 * Provides a wrapper for entities which eases dealing with entity properties.
 *
 * The wrapper eases applying getter and setter callbacks of entity properties
 * specified in hook_entity_info() while respecting the specified options.
 */
class EntityWrapper extends EntityMetadataWrapper implements IteratorAggregate {

  protected $entityInfo;
  protected $langcode = NULL;

  /**
   * Construct a new EntityWrapper object.
   *
   * @param $type
   *   The type of the passed data.
   * @param $data
   *   Optional. The data to wrap.
   * @param $info
   *   Optional. Used internally to pass info about properties down the tree.
   */
  public function __construct($type, &$data = NULL, $info = NULL) {
    parent::__construct($type, $data, $info);
    $this->entityInfo = entity_get_info($type) + array('name property' => 'name');
    $this->entityInfo = array_merge_recursive($this->entityInfo, entity_metadata_get_info($type)) + array('properties' => array());

    // Add in properties from the bundle, for that we might need to load the
    // entity if not done yet.
    if (!empty($this->entityInfo['fieldable']) && empty($info['bundle']) && $this->dataAvailable()) {
      list($id, $vid, $bundle) = entity_extract_ids($type, $this->value());
      $info['bundle'] = $bundle;
    }
    if (isset($info['bundle'])) {
      $bundle_info = (array)$this->entityInfo['bundles'][$info['bundle']] + array('properties' => array());
      $this->entityInfo['properties'] += $bundle_info['properties'];
    }
  }

  /**
   * Gets the info about the given property.
   *
   * @param $name
   *   The name of the property.
   * @throws EntityMetadataWrapperException
   *   If there is no such property.
   * @return
   *   An array of info about the property.
   */
  public function getPropertyInfo($name) {
    if (!isset($this->entityInfo['properties'][$name])) {
      throw new EntityMetadataWrapperException('Unknown entity property ' . check_plain($name) . '.');
    }
    $info = $this->entityInfo['properties'][$name] + array(
      'type' => 'text',
      'bundle' => NULL,
    );
    return $info;
  }

  /**
   * Gets the info about the wrapped entity.
   */
  public function getEntityInfo() {
    return $this->entityInfo;
  }

  /**
   * Sets a new language to use for retrieving properties.
   *
   * @param $langcode
   *   The language code of the language to set.
   * @return EntityWrapper
   */
  public function language($langcode = NULL) {
    if ($langcode != $this->langcode) {
      $this->langcode = $langcode;
      $this->cache = array();
    }
  }

  /**
   * Gets the language used for retrieving properties.
   *
   * @return String
   *   The language code of the language or NULL for the default language.
   *
   * @see EntityWrapper::language()
   */
  public function getPropertyLanguage() {
    return $this->langcode;
  }

  /**
   * Get the wrapper for a property.
   *
   * @return
   *   An instance of EntityMetadataWrapper.
   */
  public function get($name) {
    // Look it up in the cache if possible.
    if (!array_key_exists($name, $this->cache)) {
      if ($info = $this->getPropertyInfo($name)) {
        $info += array('parent' => $this, 'name' => $name, 'langcode' => $this->langcode);
        $this->cache[$name] = entity_metadata_wrapper($info['type'], $data = NULL, $info);
      }
      else {
        throw new EntityMetadataWrapperException('There is no property ' . check_plain($name) . " for this entity.");
      }
    }
    return $this->cache[$name];
  }

  /**
   * Magic method: Get a wrapper for a property.
   */
  public function __get($name) {
    return $this->get($name);
  }

  /**
   * Magic method: Set a property.
   */
  public function __set($name, $value) {
    $this->get($name)->set($value);
  }

  /**
   * Magic method: Can be used to check if a property is known.
   */
  public function __isset($name) {
    return isset($this->entityInfo['properties'][$name]);
  }

  public function getIterator() {
    return new ArrayIterator(array_keys($this->entityInfo['properties']));
  }
}

/**
 * Wraps a list of values.
 *
 * If the wrapped data is a list of data, its numerical
 * indexes may be used to retrieve wrappers for the list items. For that this
 * wrapper may be also used like a usual numerically indexed array.
 */
class EntityListWrapper extends EntityMetadataWrapper implements IteratorAggregate, ArrayAccess {

  protected $data = NULL;

  /**
   * Get the wrapper for a single item.
   *
   * @return
   *   An instance of EntityMetadataWrapper.
   */
  public function get($delta) {
    // Look it up in the cache if possible.
    if (!array_key_exists($delta, $this->cache)) {
      if ($this->dataAvailable() && isset($this->data[$delta])) {
        $info = array('parent' => $this, 'delta' => $delta) + $this->info;
        $type = entity_metadata_list_extract_type($this->type);
        $this->cache[$delta] = new EntityListItemWrapper($type, $data = NULL, $info);
      }
      else {
        throw new EntityMetadataWrapperException('There is no item with a delta of ' . check_plain($delta) . " for this list.");
      }
    }
    return $this->cache[$delta];
  }

  /**
   * If we wrap a list, we return an iterator over the data list.
   */
  public function getIterator() {
    return $this->dataAvailable() ? new ArrayIterator(array_keys($this->data)) : array();
  }

  /**
   * Implements the ArrayAccess interface.
   */
  public function offsetGet($delta) {
    return $this->get($delta);
  }

  public function offsetExists($delta) {
    return is_numeric($delta) && $this->dataAvailable() && isset($this->data[$delta]);
  }

  public function offsetSet($delta, $value) {
    if (is_numeric($delta) && $this->dataAvailable()) {
      $this->data[$delta] = $value;
      $this->set($this->data);
    }
  }

  public function offsetUnset($delta) {
    if ($this->offsetExists($delta)) {
      unset($this->data[$delta]);
    }
  }
}

/**
 * Wraps a list entry.
 */
class EntityListItemWrapper extends EntityValueWrapper {

  public function value(array $options = array()) {
    if (!isset($this->data)) {
      // Get the list from the parent then retrieve the entry.
      $list = $this->info['parent']->value();
      if (is_array($list) && isset($list[$this->info['delta']])) {
        $this->data = $list[$this->info['delta']];
      }
      else {
        throw new EntityMetadataWrapperException('List entry ' . check_plain($this->info['delta']) . " isn't set.");
      }
    }
    return $this->applyOptions($this->data, $options);
  }

  public function raw() {
    $list = $this->info['parent']->raw();
    if (is_array($list) && isset($list[$this->info['delta']])) {
      return $list[$this->info['delta']];
    }
    throw new EntityMetadataWrapperException('List entry ' . check_plain($this->info['delta']) . " isn't set.");
  }

  /**
   * Set a new data value.
   */
  public function set($value) {
    $this->info['parent']->__set($this->info['delta'], $value);
    return $this;
  }
}

/**
 * Provide a separate Exception so it can be caught separately.
 */
class EntityMetadataWrapperException extends Exception { }

