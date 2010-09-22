<?php
// $Id$

/**
 * @file
 * Provides Entity metadata integration for entities provided via the CRUD API.
 */

/**
 * Default controller for generating Rules integration.
 */
class EntityDefaultMetadataController {

  protected $type, $info;

  public function __construct($type) {
    $this->type = $type;
    $this->info = entity_get_info($type);
  }

  public function entityPropertyInfo() {
    $info['properties'] = entity_metadata_convert_schema($this->info['base table']);
    foreach ($info['properties'] as $name => &$property) {
      // Try to come up with better default labels.
      $property['label'] = t('@entity @property', array('@entity' => $this->info['label'], '@property' => $name));
    }
    return array($this->type => $info);
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
