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
 * - 'entity keys' An array of keys as defined by core. The following additional
 *   keys are used by the entity CRUD API:
 *   - name: An optional name of a property that contains a unique name of the
 *     entity. If specified, this is used as uniform identifier of the entity
 *     while the 'id' key is only used to refer to the entity internally, e.g.
 *     in the database. If not specified, this defaults to the 'id' key.
 *     For exportable entities, it's strongly recommended to use a machine name
 *     here as those are more portable across systems.
 *   - module: Optional. A key for the module property containing the source
 *     module name for exportable entities provided in code. Defaults to
 *     'module'.
 *   - status: Optional. The name of the entity property to use for setting the
 *     exportable entity status using defined bit flags. Defaults to 'status'.
 * - export: An array of optional information used for exporting. For ctools
 *   exportables compatibility any export-keys supported by ctools may be added
 *   to this array too.
 *   - default hook: What hook to invoke to find exportable entities that are
 *     currently defined. This hook is automatically called by the CRUD
 *     controller during entity_load(). Defaults to 'default_' . $entity_type.
 * - admin ui: An array of optional information used for providing an
 *   administrative user interface. To enable the UI at least the path must be
 *   given and the 'ENTITY_TYPE_form' for editing, adding and cloning has to be
 *   created. See entity_ui_get_form() for details on which parameters are
 *   passed to that form.
 *   Known keys are:
 *   - path: a path where the UI should show up as expected by hook_menu().
 *   - controller class: An optional controller class name for providing the
 *     UI. Defaults to EntityDefaultUIController.
 *     For customizing the UI inherit from the default class and overide methods
 *     as suiting and specify your class as controller class.
 *   - file: Optionally, the name of the file in which the entity form resides
 *     as it is required by hook_menu().
 *   - file path: Optionally, the path to the file as required by hook_menu. If
 *     not set, it defaults to entity module's path, thus the entity types
 *     'module' key is required.
 * - 'rules controller class': An optional controller class for providing Rules
 *   integration. The given class has to inherit from the default class being
 *   EntityDefaultRulesController. Set it to FALSE to disable this feature.
 * - 'metadata controller class': A controller class for providing Entity meta-
 *   data module integration, i.e. entity property info. By default some meta-
 *   data is generated from your hook_schema() information and *read access* is
 *   granted to that properties. From that the Entity metadata module also
 *   generates token integration for you, once activated.
 *   Override the controller class to adapt the defaults and to improve and
 *   complete the generated metadata. Set it to FALSE to disable this feature.
 *   Defaults to the EntityDefaultMetadataController class.
 * - 'features controller class': A controller class for providing Features
 *   module integration for exportable entities. The given class has to inherit
 *   from the default class being EntityDefaultFeaturesController. Set it to
 *   FALSE to disable this feature.
 * - access callback: Specify a callback that returns access permissions for the
 *   operations 'create', 'updated', 'delete' and 'view'. The callback gets
 *   optionally the entity and the user account to check for passed. See
 *   entity_metadata_no_hook_node_access() for an example.
 *   Optional, but suggested for the Rules integration.
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
