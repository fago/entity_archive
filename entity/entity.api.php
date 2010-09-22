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
 * - entity class: Optionally a class the controller will use for instantiating
 *   entities.
 * - 'bundle of': Optionally if the entity describes bundles of another entity
 *   specify the entity, for which this is a bundle of, here. If done so, the
 *   API will automatically invoke the field API bundle attachers. For this to
 *   work you also have to set the bundle key for the referred entity.
 * - module: The module providing the entity. Optionally, but suggested.
 * - exportable: Whether the entity is exportable. Defaults to FALSE.
 * - 'entity keys' - 'name': An optional name of a property that contains a
 *   unique name of the entity. If specified, this is used as uniform identifier
 *   of the entity while the 'id' key is only used to refer to the entity
 *   internally, e.g. in the database. If not specified, this defaults to the
 *   'id' key.
 *   However for exportable entities, it's suggested to use a string here as
 *   strings as names are more portable across systems. It's possible to go with
 *   the numeric 'id', but be aware that export collisions are very likely.
 * - export: An array of optional information used for exporting. Known keys
 *   are:
 *   - default hook: What hook to invoke to find exportable objects that are
 *     currently defined. These will all be gathered into a giant array.
 *     Defaults to 'default_' . $entity_type.
 *   - status key: The name of the entity property to use for setting the
 *     entities status using defined bit flags. Defaults to 'status'.
 *   - identifier: When exporting the object, the identifier is the variable
 *     that the exported object will be placed in. Defaults to the entity type.
 *   - bulk export': Declares whether or not the exportable will be available
 *     for bulk exporting.
 *   - export callback: The callback to use for bulk exporting. Defaults to
 *     $module . '_export_' . $entity_type.
 *   - list callback: Bulk export callback to provide a list of exportable
 *     objects to be chosen for bulk exporting. Defaults to
 *     $module . '_' . $entity_type . '_list'.
 *   - to hook code callback: Function used to generate an export for the bulk
 *     export process. This is only necessary if the export is more complicated
 *     than simply listing the fields. Defaults to $module . '_' .
 *     $entity_type . '_to_hook_code'.
 * - 'rules controller class': An optional controller class for providing Rules
 *   integration. The given class has to inherit from the default class being
 *   EntityDefaultRulesController.
 * - 'metadata controller class': A controller class for providing Entity meta-
 *   data module integration, i.e. entity property info. By default some meta-
 *   data is generated from your hook_schema() information and *read access* is
 *   granted to that properties. From that the Entity metadata module also
 *   generates token integration for you, once activated.
 *   Override the controller class to adapt the defaults and to improve and
 *   complete the generated metadata. Defaults to the
 *   EntityDefaultMetadataController class.
 *
 * @see hook_entity_info()
 */
function entity_hook_entity_info() {
  $return = array(
    'entity_test' => array(
      'label' => t('Test Entity'),
      'entity class' => 'EntityDB',
      'controller class' => 'EntityAPIController',
      'base table' => 'entity_test',
      'module' => 'entity_test',
      'fieldable' => TRUE,
      'entity keys' => array(
        'id' => 'pid',
        'name' => 'name',
        'bundle' => 'type',
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
