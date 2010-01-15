<?php
// $Id$

/**
 * @file
 * Info include file. Includes further module.info.inc files for core modules,
 * for which we provide metadata.
 */


function _entity_metadata_core_modules() {
  return array_filter(array('book', 'comment', 'field', 'node', 'taxonomy', 'user', 'system'), 'module_exists');
}

/**
 * Implements hook_entity_property_info().
 */
function entity_metadata_entity_property_info() {
  $items = array();
  foreach (_entity_metadata_core_modules() as $module) {
    module_load_include('inc', 'entity_metadata', "modules/$module.info");
    if (function_exists($function = "entity_metadata_{$module}_entity_property_info")) {
      if ($return = $function()) {
        $items = array_merge_recursive($items, $return);
      }
    }
  }
  return $items;
}


/**
 * Implements hook_entity_property_info_alter().
 */
function entity_metadata_entity_property_info_alter(&$entity_info) {
  foreach (_entity_metadata_core_modules() as $module) {
    module_load_include('inc', 'entity_metadata', "modules/$module.info");
    if (function_exists($function = "entity_metadata_{$module}_entity_property_info_alter")) {
      $function($entity_info);
    }
  }
}