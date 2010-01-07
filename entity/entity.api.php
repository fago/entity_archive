<?php
// $Id$

/**
 * @file
 * Hooks provided by the entity API.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * This is just placeholder for describing further keys for describing entities
 * in hook_entity_info(), which are introduced by the entity API:
 * - class: The class to use when loading entities with the provided controller
 *   class EntityCRUDController. Optionally, but to have full CRUD functionality
 *   the EntityDB class provided by the entity API or a custom class which
 *   extends EntityDB is suggested.
 * - exportable: Whether the entity is exportable. Defaults to FALSE.
 * - export: An array of optional information used for exporting. Known keys
 *   are:
 *   - key: The key used to identify the exported entity. By the default the
 *     entity identifier is used, but it's overridden by this setting.
 *     It's suggested to use a string here as string as names are more portable
 *     across systems. It is possible to use numbers, but be aware that export
 *     collisions are very likely.
 *   - can disable: Control whether or not the exportable objects can be
 *     disabled. All this does is cause the 'disabled' field on the object
 *     to always be set appropriately, and a variable is kept to record
 *     the state. Changes made to this state must be handled by the owner
 *     of the object. Defaults to TRUE.
 *   - status: Exportable objects can be enabled or disabled, and this status
 *     is stored in a variable. This defines what variable that is. Defaults
 *     to: 'default_' . $table,
 *   - default hook: What hook to invoke to find exportable objects that are
 *     currently defined. These will all be gathered into a giant array.
 *     Defaults to 'default_' . $table,
 *   - identifier: When exporting the object, the identifier is the variable that
 *     the exported object will be placed in. Defaults to $table.
 *   - bulk export': Declares whether or not the exportable will be available
 *     for bulk exporting. (Bulk export UI is currently left to be handled
 *     by contrib, though the core hooks are present so this can be done.).
 *   - export callback: The callback to use for bulk exporting. Defaults to
 *     $module . '_export_' . $table.
 *   - list callback: Bulk export callback to provide a list of exportable
 *     objects to be chosen for bulk exporting. Defaults to
 *     $module . '_' . $table . '_list'.
 *   - to hook code callback: Function used to generate an export for the bulk
 *     export process. This is only necessary if the export is more complicated
 *     than simply listing the fields. Defaults to $module . '_' . $table .
 *     '_to_hook_code'.
 */
function entity_hook_entity_info() {
  $return = array(
    'entity_test' => array(
      'label' => t('Test Entity'),
      'entity class' => 'EntityClass',
      'controller class' => 'EntityCRUDController',
      'base table' => 'entity_test',
      'fieldable' => TRUE,
      'object keys' => array(
        'id' => 'pid',
        'bundle' => 'name',
      ),
      'bundles' => array(),
    ),
  );
  foreach (entity_test_get_types() as $name => $info) {
    $return['entity_test']['bundles'][$name] = array(
      'label' => $info['label'],
    );
  }
  return $return;
}

/**
 * @} End of "addtogroup hooks".
 */
