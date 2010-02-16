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
      'langcode' => NULL,
    );
    $this->info['type'] = $type;
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
   * Returns the wrapped data. If no options are given the data is returned as
   * described in the info.
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
      $this->data =& $this->info['parent']->getPropertyValue($this->info['name'], $this->info);
    }
    elseif (!isset($this->data)) {
      throw new EntityMetadataWrapperException('There is no data set, so you cannot retrieve any values.');
    }
    return $this->data;
  }

  /**
   * Returns the raw, unprocessed data. Most times this is the same as returned
   * by value(), however for already processed and sanitized textual data, this
   * will return the unprocessed data in contrast to value().
   */
  public function raw() {
    if (isset($this->info['name']) && isset($this->info['parent'])) {
      return $this->info['parent']->getPropertyRaw($this->info['name'], $this->info);
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
      $this->data =& $this->info['parent']->getPropertyValue($this->info['name'], $this->info);
    }
    return isset($this->data);
  }

  /**
   * Set a new data value.
   */
  public function set($value) {
    if (empty($this->info['parent'])) {
      $this->clear();
      $this->data = $value;
    }
    else {
      $this->info['parent']->setProperty($this->info['name'], $value);
    }
    return $this;
  }

  public function __toString() {
    return isset($this->info) ? 'Property ' . $this->info['name'] : $this->type;
  }

  /**
   * Clears the data value and the wrapper cache.
   */
  protected function clear() {
    $this->data = NULL;
    foreach ($this->cache as $wrapper) {
      $wrapper->clear();
    }
  }

  /**
   * Returns the options list specifying possible key value pairs for the
   * property, if defined.
   *
   * @return
   *   An array as used by hook_options_list() or FALSE.
   */
  public function optionsList() {
    if (isset($this->info['options list']) && is_callable($this->info['options list']) && $this->dataAvailable() && isset($this->info['name'])) {
      return call_user_func($this->info['options list'], $this->value(), $this->info['name'], $this->type);
    }
    return FALSE;
  }

  /**
   * Determines whether the given user has access to view or edit this property.
   * Apart from relying on access metadata of properties, this takes into
   * account information about entity level access, if available:
   *  - Referenced entities can only be viewed, when the user also has
   *    permission to view the entity.
   *  - A property may be only edited, if the user has permission to update the
   *    entity containing the property.
   *
   * @param $op
   *   The operation being performed. One of 'view' or 'edit.
   * @param $account
   *   The user to check for. Leave it to NULL to check for the global user.
   * @return boolean
   *   Whether access to entity property is allowed for the given operation.
   *   However if we wrap no data, it returns whether access is allowed to the
   *   property of all entities of this type.
   *   If there is no access information for this property, TRUE is returned.
   */
  public function access($op, $account = NULL) {
    return !empty($this->info['parent']) ? $this->info['parent']->propertyAccess($this->info['name'], $op, $account) : TRUE;
  }
}

/**
 * Wraps a single value.
 */
class EntityValueWrapper extends EntityMetadataWrapper {

  /**
   * Overrides EntityMetadataWrapper#value().
   * Sanitizes or decode textual data if necessary.
   */
  public function value(array $options = array()) {
    $data = parent::value();
    if ($this->type == 'text') {
      $info = $this->info + array('sanitized' => FALSE, 'sanitize' => 'check_plain');
      $options += array('sanitize' => FALSE, 'decode' => FALSE);
      if ($options['sanitize'] && !$info['sanitized']) {
        return call_user_func($info['sanitize'], $data);
      }
      elseif ($options['decode'] && $info['sanitized']) {
        return decode_entities(strip_tags($data));
      }
    }
    return $data;
  }
}

/**
 * Provides a general wrapper for any data structure. For this to work the
 * metadata has to be passed during construction.
 */
class EntityStructureWrapper extends EntityMetadataWrapper implements IteratorAggregate {

  protected $dataInfo;
  protected $langcode = NULL;

  protected $propertyInfoDefaults = array(
    'type' => 'text',
    'getter callback' => 'entity_metadata_verbatim_get',
    'clear' => array(),
  );

  /**
   * Construct a new EntityStructureWrapper object.
   *
   * @param $type
   *   The type of the passed data.
   * @param $data
   *   Optional. The data to wrap.
   * @param $info
   *   Used to for specifying metadata about the data and internally to pass
   *   info about properties down the tree. For specifying metadata known keys
   *   are:
   *   - data info: An array of info about the wrapped data. It has to contain
   *     an array of info about the properties of the data, in the same
   *     structure as used by hook_entity_property_info().
   */
  public function __construct($type, &$data = NULL, $info = NULL) {
    parent::__construct($type, $data, $info);
    $info += array('data info' => array());
    $this->dataInfo = $info['data info'] + array('properties' => array());
  }

  /**
   * May be used to lazy-load additional info about the data, depending on the
   * concrete passed data.
   */
  protected function spotInfo() {
    // May be implemented by child classes.
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
    $this->spotInfo();
    $name = str_replace('_', '-', $name);
    if (!isset($this->dataInfo['properties'][$name])) {
      throw new EntityMetadataWrapperException('Unknown data property ' . check_plain($name) . '.');
    }
    return $this->dataInfo['properties'][$name] + $this->propertyInfoDefaults;
  }

  /**
   * Gets the info about the wrapped data.
   */
  public function dataInfo() {
    $this->spotInfo();
    return $this->dataInfo;
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
   *   The language object of the language or NULL for the default language.
   *
   * @see EntityStructureWrapper::language()
   */
  public function getPropertyLanguage() {
    if (isset($this->langcode) && $list = language_list()) {
      return $list[$this->langcode];
    }
    return NULL;
  }

  /**
   * Get the wrapper for a property.
   *
   * @return
   *   An instance of EntityMetadataWrapper.
   */
  public function get($name) {
    $name = str_replace('_', '-', $name);
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
   * Gets the value of a property.
   */
  protected function &getPropertyValue($name, &$info) {
    if ($this->dataAvailable()) {
      $options = array('language' => $this->getPropertyLanguage(), 'absolute' => TRUE);
      $value =& $info['getter callback']($this->value(), $options, $name, $this->type, $info);
      return $value;
    }
    throw new EntityMetadataWrapperException('Entity property ' . check_plain($name) . " isn't set.");
  }

  /**
   * Gets the raw value of a property.
   */
  protected function getPropertyRaw($name, &$info) {
    if (!empty($info['raw getter callback']) && $this->dataAvailable()) {
      $options = array('language' => $this->getPropertyLanguage(), 'absolute' => TRUE);
      return $info['raw getter callback']($this->value(), $options, $name, $this->type, $info);
    }
    return $this->getPropertyValue($name, $info);
  }

  /**
   * Sets a property.
   */
  protected function setProperty($name, $value) {
    $name = str_replace('_', '-', $name);
    $info = $this->getPropertyInfo($name);
    if (!empty($info['setter callback']) && $this->dataAvailable()) {
      $info['setter callback']($this->value(), $name, $value, $this->langcode, $this->type);
      $this->get($name)->clear();
      // Clear the cache of properties dependent on this value.
      foreach ($info['clear'] as $name) {
        if (isset($this->cache[$name])) {
          $this->cache[$name]->clear();
        }
      }
    }
    else {
      throw new EntityMetadataWrapperException('Entity property ' . check_plain($name) . " doesn't support writing.");
    }
  }

  protected function propertyAccess($name, $op, $account = NULL) {
    $name = str_replace('_', '-', $name);
    $info = $this->getPropertyInfo($name);
    // If the property should be accessed and it's an entity, make sure the user
    // is allowed to view that entity.
    if ($op == 'view' && $this->$name instanceof EntityDrupalWrapper && !$this->$name->entityAccess($op, $account)) {
      return FALSE;
    }
    // If a property should be edited and this is an entity, make sure the user
    // has update access for this entity.
    if ($op == 'edit') {
      $entity = $this;
      while (!($entity instanceof EntityDrupalWrapper) && isset($entity->info['parent'])) {
        $entity = $entity->info['parent'];
      }
      if ($entity instanceof EntityDrupalWrapper && !$entity->entityAccess('update', $account)) {
        return FALSE;
      }
    }
    if (!empty($info['access callback'])) {
      $data = $this->dataAvailable() ? $this->value() : NULL;
      return call_user_func($info['access callback'], $op, $name, $this->type, $data, $account);
    }
    elseif ($op == 'edit' && isset($info['setter permission'])) {
      return user_access($info['setter permission'], $account);
    }
    return TRUE;
  }

  /**
   * Magic method: Can be used to check if a property is known.
   */
  public function __isset($name) {
    $this->spotInfo();
    $name = str_replace('_', '-', $name);
    return isset($this->dataInfo['properties'][$name]);
  }

  public function getIterator() {
    $this->spotInfo();
    return new ArrayIterator($this->dataInfo['properties']);
  }

  /**
   * Returns the identifier of the data structure. If there is none, NULL is
   * returned.
   */
  public function getIdentifier() {
    return isset($this->id) && $this->dataAvailable() ? $this->id->value() : NULL;
  }
}

/**
 * Provides a wrapper for entities registrered in hook_entity_info().
 *
 * The wrapper eases applying getter and setter callbacks of entity properties
 * specified in hook_entity_property_info().
 */
class EntityDrupalWrapper extends EntityStructureWrapper {

  /**
   * Contains the fully loaded entity while $data only contains the identifier.
   */
  protected $entity = FALSE;

  /**
   * Construct a new EntityDrupalWrapper object.
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
    $this->dataInfo = entity_get_info($type) + array('name property' => 'name');
    $this->dataInfo = array_merge_recursive($this->dataInfo, entity_metadata_get_info($type)) + array('properties' => array());
    if (!empty($this->info['bundle'])) {
      $this->spotInfo($this->info['bundle']);
    }
    $this->setEntity($data);
  }

  /**
   * Sets the entity internally accepting both the entity id and object.
   */
  protected function setEntity($data) {
    if ($data && !is_object($data)) {
      $this->data = $data;
      $this->entity = FALSE;
    }
    elseif ($data) {
      $this->entity = $data;
      list($id, $vid, $bundle) = entity_extract_ids($this->type, $data);
      $this->data = isset($id) ? $id : FALSE;
    }
  }

  /**
   * Used to lazy-load bundle info. So the wrapper can be loaded e.g. just
   * for setting without the data being loaded.
   */
  protected function spotInfo($bundle = NULL) {
    if (!isset($this->info['bundle']) || isset($bundle)) {
      $this->info['bundle'] = $bundle;
      // Add in properties from the bundle, for that we might need to load the
      // entity if not done yet.
      if (!isset($this->info['bundle']) && !empty($this->dataInfo['fieldable']) && $this->dataAvailable()) {
        list($id, $vid, $bundle) = entity_extract_ids($this->type, $this->value());
        $this->info['bundle'] = $bundle;
      }
      if ($this->info['bundle'] && isset($this->dataInfo['bundles'][$this->info['bundle']])) {
        $bundle_info = (array)$this->dataInfo['bundles'][$this->info['bundle']] + array('properties' => array());
        $this->dataInfo['properties'] += $bundle_info['properties'];
      }
    }
  }

  /**
   * Returns the identifier of the wrapped entity.
   */
  public function getIdentifier() {
    return $this->dataAvailable() ? $this->data : NULL;
  }

  /**
   * Overridden.
   */
  public function value() {
    if (!$this->dataAvailable()) {
      throw new EntityMetadataWrapperException('There is no data set, so you cannot retrieve any values.');
    }
    elseif (!$this->entity && $this->data) {
      // Lazy load the entity if necessary.
      $this->entity = reset(entity_load($this->type, array($this->data)));
      if (!$this->entity) {
        throw new EntityMetadataWrapperException('Unable to load the ' . check_plain($this->type) . ' with the id ' . check_plain($this->data) . '.');
      }
    }
    return $this->entity;
  }

  /**
   * Overridden to support setting the entity by object and id.
   */
  protected function dataAvailable() {
    if (!isset($this->data) && isset($this->info['name']) && parent::dataAvailable()) {
      $this->setEntity($this->data);
    }
    return isset($this->data);
  }

  /**
   * Overridden to support setting the entity by either the object or the id.
   */
  public function set($value) {
    // Let the setter get the identifier but accept both.
    $this->setEntity($value);
    return parent::set($this->data);
  }

  /**
   * Overridden.
   */
  public function clear() {
    $this->entity = FALSE;
    parent::clear();
  }

  /**
   * Checks whether the operation $op is allowed on the entity.
   *
   * @see entity_metadata_entity_access()
   */
  public function entityAccess($op, $account = NULL) {
    $entity = $this->dataAvailable() ? $this->value() : NULL;
    return entity_metadata_entity_access($op, $this->type, $entity, $account);
  }
}

/**
 * Wraps a list of values.
 *
 * If the wrapped data is a list of data, its numerical indexes may be used to
 * retrieve wrappers for the list items. For that this wrapper implements
 * ArrayAccess so it may be used like a usual numerically indexed array.
 */
class EntityListWrapper extends EntityMetadataWrapper implements IteratorAggregate, ArrayAccess, Countable {

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
      if (!isset($delta)) {
        // The [] operator has been used so point at a new entry.
        $delta = max(array_keys($this->data)) + 1;
      }
      if (is_numeric($delta)) {
        $info = array('parent' => $this, 'name' => $delta) + $this->info;
        $type = entity_metadata_list_extract_type($this->type);
        $this->cache[$delta] = entity_metadata_wrapper($type, NULL, $info);
      }
      else {
        throw new EntityMetadataWrapperException('There can be only numerical keyed items in a list.');
      }
    }
    return $this->cache[$delta];
  }

  protected function &getPropertyValue($delta) {
    if ($this->dataAvailable() && isset($this->data[$delta])) {
      return $this->data[$delta];
    }
    throw new EntityMetadataWrapperException('There is no item with a delta of ' . check_plain($delta) . " for this list.");
  }

  protected function getPropertyRaw($delta) {
    $list = $this->raw();
    if (isset($list[$delta])) {
      return $list[$delta];
    }
    throw new EntityMetadataWrapperException('List entry ' . check_plain($delta) . " isn't set.");
  }

  protected function setProperty($delta, $value) {
    if (is_numeric($delta) && $this->dataAvailable()) {
      $this->data[$delta] = $value;
      $this->set($this->data);
    }
  }

  protected function propertyAccess($delta, $op, $account = NULL) {
    return $this->access($op, $account);
  }

  /**
   * @param $options
   *   An array of options. Known keys:
   *   identifier: If set to TRUE for a list of entities, it won't be returned
   *   as list of fully loaded entity objects, but as a list of entity ids.
   */
  public function value(array $options = array()) {
    // For lists of entities fetch full entity objects before returning.
    if ($this->get(0) instanceof EntityDrupalWrapper && empty($options['identifier']) && $this->dataAvailable()) {
      return array_values(entity_load($this->get(0)->type, $this->data));
    }
    return parent::value();
  }

  public function set($values) {
    // Support setting lists of fully loaded entities.
    if ($this->get(0) instanceof EntityDrupalWrapper && !empty($values[0]) && is_object($values[0])) {
      foreach ($values as $key => $value) {
        list($id, $vid, $bundle) = entity_extract_ids($this->get(0)->type, $value);
        $values[$key] = $id;
      }
    }
    return parent::set($values);
  }

  /**
   * If we wrap a list, we return an iterator over the data list.
   */
  public function getIterator() {
    return $this->dataAvailable() ? new ArrayIterator($this->value()) : array();
  }

  /**
   * Implements the ArrayAccess interface.
   */
  public function offsetGet($delta) {
    return $this->get($delta);
  }

  public function offsetExists($delta) {
    return $this->dataAvailable() && isset($this->data[$delta]);
  }

  public function offsetSet($delta, $value) {
    $this->get($delta)->set($value);
  }

  public function offsetUnset($delta) {
    if ($this->offsetExists($delta) && $this->dataAvailable()) {
      unset($this->data[$delta]);
      $this->set($this->data);
    }
  }

  public function count() {
    return $this->dataAvailable() ? count($this->data) : 0;
  }
}

/**
 * Provide a separate Exception so it can be caught separately.
 */
class EntityMetadataWrapperException extends Exception { }

