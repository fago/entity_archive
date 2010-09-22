<?php
// $Id$

/**
 * @file
 * Provides Entity metadata integration for entities provided via the CRUD API.
 */

/**
 * Default controller for generating some basic metadata.
 */
class EntityDefaultMetadataController {

  protected $type, $info;

  public function __construct($type) {
    $this->type = $type;
    $this->info = entity_get_info($type);
  }

  public function entityPropertyInfo() {
    $entity_label = drupal_strtolower($this->info['label']);

    // Provide defaults based on the schema.
    $info['properties'] = entity_metadata_convert_schema($this->info['base table']);
    foreach ($info['properties'] as $name => &$property) {
      // Add a description.
      $property['description'] = t('@entity "@property" property.', array('@entity' => drupal_ucfirst($entity_label), '@property' => $name));
    }

    // Set better metadata for known entity keys.
    $id_key = $this->info['entity keys']['id'];
    if (!empty($this->info['entity keys']['name']) && $key = $this->info['entity keys']['name']) {
      $info['properties'][$key]['type'] = 'token';
      $info['properties'][$key]['label'] = t('Machine-readable name');
      $info['properties'][$key]['description'] = t('The machine-readable name identifying this @entity.', array('@entity' => $entity_label));
      $info['properties'][$id_key]['label'] = t('Internal, numeric @entity ID', array('@entity' => $entity_label));
      $info['properties'][$id_key]['description'] = t('The ID used to identify this @entity internally.', array('@entity' => $entity_label));
    }
    else {
      $info['properties'][$id_key]['label'] = t('@entity ID', array('@entity' => drupal_ucfirst($entity_label)));
      $info['properties'][$id_key]['description'] = t('The unique ID of the @entity.', array('@entity' => $entity_label));
    }
    // Care for the bundle.
    if (!empty($this->info['entity keys']['bundle']) && $key = $this->info['entity keys']['bundle']) {
      $info['properties'][$key]['type'] = 'token';
      $info['properties'][$key]['options list'] = array(get_class($this), 'bundleOptionsList');
    }
    // Care for the label.
    if (!empty($this->info['entity keys']['label']) && $key = $this->info['entity keys']['label']) {
      $info['properties'][$key]['label'] = t('Label');
      $info['properties'][$key]['description'] = t('The human readable label.');
    }
    return array($this->type => $info);
  }

  /**
   * A options list callback returning all bundles for an entity type.
   */
  public static function bundleOptionsList($name, $info) {
    if (!empty($info['parent']) && $type = $info['parent']) {
      $entity_info = $info['parent']->entityInfo();
      $options = array();
      foreach ($entity_info['bundles'] as $name => $bundle_info) {
        $options[$name] = $bundle_info['label'];
      }
      return $options;
    }
  }
}

/**
 * Implements hook_entity_property_info().
 */
function entity_entity_property_info() {
  $items = array();
  foreach (entity_crud_get_info() as $type => $info) {
    $info += array('metadata controller class' => 'EntityDefaultMetadataController');
    $controller = new $info['metadata controller class']($type);
    $items += $controller->entityPropertyInfo();
  }
  return $items;
}
