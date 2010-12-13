<?php
// $Id$

/**
 * @file
 * Provides API functions around hook_entity_property_info(). Also see
 * entity.info.inc, which cares for providing entity property info for all core
 * entity types.
 */

/**
 * Get the entity property info array of an entity type.
 *
 * @see hook_entity_property_info()
 * @see hook_entity_property_info_alter()
 *
 * @param $entity_type
 *   The entity type, e.g. node, for which the info shall be returned, or NULL
 *   to return an array with info about all types.
 */
function entity_get_property_info($entity_type = NULL) {
  // Use the advanced drupal_static() pattern, since this is called very often.
  static $drupal_static = array();
  isset($drupal_static[__FUNCTION__]) || ($drupal_static[__FUNCTION__] = &drupal_static(__FUNCTION__));
  $info = &$drupal_static[__FUNCTION__];

  // hook_entity_property_info() includes translated strings, so each language
  // is cached separately.
  $langcode = $GLOBALS['language']->language;

  if (empty($info)) {
    if ($cache = cache_get("entity_property_info:$langcode")) {
      $info = $cache->data;
    }
    else {
      $info = module_invoke_all('entity_property_info');
      // Let other modules alter the entity info.
      drupal_alter('entity_property_info', $info);
      cache_set("entity_property_info:$langcode", $info);
    }
  }
  return empty($entity_type) ? $info : (isset($info[$entity_type]) ? $info[$entity_type] : array());
}

/**
 * Gets an array of info about all properties of a given entity type.
 *
 * In contrast to entity_get_property_info(), this function returns info about
 * all properties the entity might have, thus it adds an all properties assigned
 * to entity bundles.
 *
 * @param $entity_type
 *   (optiona) The entity type to return properties for.
 *
 * @return
 *   An array of info about properties. If the type is ommitted, all known
 *   properties are returned.
 */
function entity_get_all_property_info($entity_type = NULL) {
  if (!isset($entity_type)) {
    // Retrieve all known properties.
    $properties = array();
    foreach (entity_get_info() as $entity_type => $info) {
      $properties += entity_metadata_get_properties($entity_type);
    }
    return $properties;
  }
  // Else retrieve the properties of the given entity type only.
  $info = entity_metadata_get_info($entity_type);
  $info += array('properties' => array(), 'bundles' => array());
  // Add all bundle properties.
  foreach ($info['bundles'] as $bundle => $bundle_info) {
    $bundle_info += array('properties' => array());
    $info['properties'] += $bundle_info['properties'];
  }
  return $info['properties'];
}

/**
 * Resets the cached information of hook_entity_property_info().
 */
function entity_property_info_cache_clear() {
  drupal_static_reset('entity_get_property_info');
  // Clear all languages.
  cache_clear_all('entity_property_info:', 'cache', TRUE);
}

/**
 * Implements hook_hook_info().
 */
function entity_hook_info() {
  $hook_info['entity_property_info'] = array(
    'group' => 'info',
  );
  return $hook_info;
}

/**
 * Implements hook_field_info_alter().
 * Defines default property types for core field types.
 */
function entity_field_info_alter(&$field_info) {
  if (module_exists('number')) {
    $field_info['number_integer']['property_type'] = 'integer';
    $field_info['number_decimal']['property_type'] = 'decimal';
    $field_info['number_float']['property_type'] = 'decimal';
  }
  if (module_exists('text')) {
    $field_info['text']['property_type'] = 'text';
    $field_info['text']['property_callbacks'][] = 'entity_metadata_field_text_property_callback';
    $field_info['text_long']['property_type'] = 'text';
    $field_info['text_long']['property_callbacks'][] = 'entity_metadata_field_text_property_callback';
    $field_info['text_with_summary']['property_type'] = 'text';
    $field_info['text_with_summary']['property_callbacks'][] = 'entity_metadata_field_text_property_callback';
  }
  if (module_exists('list')) {
    $field_info['list']['property_type'] = 'integer';
    $field_info['list_boolean']['property_type'] = 'boolean';
    $field_info['list_number']['property_type'] = 'decimal';
    $field_info['list_text']['property_type'] = 'text';
  }
  if (module_exists('taxonomy')) {
    $field_info['taxonomy_term_reference']['property_type'] = 'taxonomy_term';
    $field_info['taxonomy_term_reference']['property_callbacks'][] = 'entity_metadata_field_term_reference_callback';
  }
}

/**
 * Implements hook_field_create_instance().
 * Clear the cache when a field instance changed.
 */
function entity_field_create_instance() {
  entity_property_info_cache_clear();
}

/**
 * Implements hook_field_delete_instance().
 * Clear the cache when a field instance changed.
 */
function entity_field_delete_instance() {
  entity_property_info_cache_clear();
}

/**
 * Implements hook_field_update_instance().
 * Clear the cache when a field instance changed.
 */
function entity_field_update_instance() {
  entity_property_info_cache_clear();
}

/**
 * Extracts the contained type for a list type string like list<date>.
 *
 * @return
 *   The contained type or FALSE, if the given type string is no list.
 */
function entity_property_list_extract_type($type) {
  if (strpos($type, 'list<') === 0 && $type[strlen($type)-1] == '>') {
    return substr($type, 5, -1);
  }
  return FALSE;
}

/**
 * Gets the property just as it is set in the data.
 */
function entity_property_verbatim_get($data, array $options, $name) {
  if ((is_array($data) || (is_object($data) && $data instanceof ArrayAccess)) && isset($data[$name])) {
    return $data[$name];
  }
  elseif (is_object($data) && isset($data->$name)) {
    return $data->$name;
  }
  return NULL;
}

/**
 * Sets the property to the given value. May be used as 'setter callback'.
 */
function entity_property_verbatim_set($data, $name, $value) {
  if (is_object($data) && $data instanceof ArrayAccess) {
    $data[$name] = $value;
  }
  elseif (is_object($data)) {
    $data->$name = $value;
  }
}

/**
 * Gets the property using the getter method (named just like the property).
 */
function entity_property_getter_method($object, array $options, $name) {
  return $object->$name();
}

/**
 * Sets the property to the given value using the setter method. May be used as
 * 'setter callback'.
 */
function entity_property_setter_method($object, $name, $value) {
  $method = 'set' . $name;
  // Invoke the setProperty() method where 'Property' is the property name.
  $object->$method($value);
}

/**
 * Getter callback for getting an array. Makes sure it's numerically indexed.
 */
function entity_property_get_list($data, array $options, $name) {
  return isset($data->$name) ? array_values($data->$name) : array();
}

/**
 * A validation callback ensuring the passed integer is positive.
 */
function entity_property_validate_integer_positive($value) {
  return $value > 0;
}

/**
 * A validation callback ensuring the passed integer is non-negative.
 */
function entity_property_validate_integer_non_negative($value) {
  return $value >= 0;
}
