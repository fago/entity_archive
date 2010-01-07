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
