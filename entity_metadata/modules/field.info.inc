<?php
// $Id$

/**
 * @file
 * Provides info for fields.
 */

/**
 * Implements hook_entity_property_info() on top of field module.
 *
 * @see entity_metadata_field_info_alter()
 */
function entity_metadata_field_entity_property_info() {
  $info = array();
  // Loop over all field instances and add them as property.
  foreach (field_info_fields() as $field_name => $field) {
    $field += array('bundles' => array());
    $field_type = field_info_field_types($field['type']) + array('property_callbacks' => array());
    // Add in our default callback as the first one.
    array_unshift($field_type['property_callbacks'], 'entity_metadata_field_default_property_callback');

    foreach ($field['bundles'] as $entity_type => $bundles) {
      foreach ($bundles as $bundle) {
        $instance = field_info_instance($entity_type, $field_name, $bundle);

        if (empty($instance['deleted'])) {
          foreach ($field_type['property_callbacks'] as $callback) {
            $callback($info, $entity_type, $field, $instance, $field_type);
          }
        }
      }
    }
  }
  return $info;
}

/**
 * Callback to add in property info defaults per field instance.
 * @see entity_metadata_field_entity_property_info_alter().
 */
function entity_metadata_field_default_property_callback(&$info, $entity_type, $field, $instance, $field_type) {
  if (!empty($field_type['property_type'])) {
    if ($field['cardinality'] != 1) {
      $field_type['property_type'] = 'list<' . $field_type['property_type'] . '>';
    }
    // Add in instance specific property info, if given and apply defaults.
    $name = str_replace('_', '-', $field['field_name']);
    $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$name];
    $instance += array('property info' => array());
    $property = $instance['property info'] + array(
      'label' => $instance['label'],
      'type' => $field_type['property_type'],
      'description' => t('Field %name', array('%name' => $name)),
      'getter callback' => 'entity_metadata_field_property_get',
      'setter callback' => 'entity_metadata_field_property_set',
      'access callback' => 'entity_metadata_field_access_callback',
      'query callback' => 'entity_metadata_field_query',
      // Specify that this property stems from a field.
      'field' => TRUE,
    );
    // For field types of the list module add in the options list callback.
    if (strpos($field['type'], 'list') === 0) {
      $property['options list'] = 'entity_metadata_field_options_list';
    }
  }
}

/**
 * Additional callback to adapt the property info for text fields. If a text
 * field is processed we make use of a separate data structure so that format
 * filters are available too. For the text value the sanitized, thus processed
 * value is returned by default.
 *
 * @see entity_metadata_field_entity_property_info_alter().
 */
function entity_metadata_field_text_property_callback(&$info, $entity_type, $field, $instance, $field_type) {
  if (!empty($instance['settings']['text_processing'])) {
    $name = str_replace('_', '-', $field['field_name']);
    $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$name];
    // Define a data structure so it's possible to deal with formatted text
    // properly. For that use the else undefined type 'text_formatted'.
    $property['type'] = ($field['cardinality'] != 1) ? 'list<text_formatted>' : 'text_formatted';
    $property['getter callback'] = 'entity_metadata_field_verbatim_get';
    $property['setter callback'] = 'entity_metadata_field_verbatim_set';
    $property['property info'] = entity_metadata_text_formatted_properties();
    unset($property['query callback']);
  }
}

/**
 * Additional callback to adapt the property info for term reference fields.
 * @see entity_metadata_field_entity_property_info_alter().
 */
function entity_metadata_field_term_reference_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $name = str_replace('_', '-', $field['field_name']);
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$name];
  $property['options list'] = 'entity_metadata_field_options_list';
  unset($property['query callback']);
}
