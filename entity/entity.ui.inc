<?php
// $Id$

/**
 * @file
 * Provides a controller for building an entity overview form.
 */

/**
 * Default controller for providing UI.
 */
class EntityDefaultUIController {

  protected $entityType;
  protected $entityInfo;

  public function __construct($entity_type, $entity_info) {
    $this->entityType = $entity_type;
    $this->entityInfo = $entity_info;
  }

  /**
   * Provides definitions for implementing hook_menu().
   */
  public function hook_menu() {
    $items = array();
    $path = $this->entityInfo['admin ui']['path'];
    $id_count = count(explode('/', $path));

    $items[$path] = array(
      'title' => $this->entityInfo['label'] . 's',
      'page callback' => 'drupal_get_form',
      'page arguments' => array($this->entityType . '_overview_form', $this->entityType),
      'description' => 'Manage ' . $this->entityInfo['label'] . 's',
      'access callback' => 'entity_access',
      'access arguments' => array('view', $this->entityType),
      'file' => 'entity.ui.inc',
    );
    $items[$this->entityInfo['admin ui']['path'] . '/add'] = array(
      'title' => 'Add',
      'page callback' => 'entity_ui_get_form',
      'page arguments' => array($this->entityType, NULL, 'add'),
      'access callback' => 'entity_access',
      'access arguments' => array('create', $this->entityType),
      'type' => MENU_LOCAL_ACTION,
      'file' => 'entity.ui.inc',
    );
    $items[$path . '/%entity_menu/edit'] = array(
      'title' => 'Edit',
      'page callback' => 'entity_ui_get_form',
      'page arguments' => array($this->entityType, $id_count),
      'load arguments' => array($this->entityType),
      'access callback' => 'entity_access',
      'access arguments' => array('update', $this->entityType, $id_count),
      'type' => MENU_CALLBACK | MENU_VISIBLE_IN_BREADCRUMB,
      'file' => 'entity.ui.inc',
    );
    // Clone form, a special case for the edit form.
    $items[$path . '/%entity_menu/clone'] = array(
      'title' => 'Clone',
      'page callback' => 'entity_ui_get_form',
      'page arguments' => array($this->entityType, $id_count, 'clone'),
      'load arguments' => array($this->entityType),
      'access callback' => 'entity_access',
      'access arguments' => array('create', $this->entityType),
      'type' => MENU_CALLBACK | MENU_VISIBLE_IN_BREADCRUMB,
      'file' => 'entity.ui.inc',
    );
    // Menu item for operations like revert and delete.
    $items[$path . '/%entity_menu/%'] = array(
      'page callback' => 'drupal_get_form',
      'page arguments' => array($this->entityType . '_operation_form', $this->entityType, $id_count, $id_count + 1),
      'load arguments' => array($this->entityType),
      'access callback' => 'entity_access',
      'access arguments' => array('delete', $this->entityType, $id_count),
      'type' => MENU_CALLBACK | MENU_VISIBLE_IN_BREADCRUMB,
      'file' => 'entity.ui.inc',
    );
    return $items;
  }

  /**
   * Provides definitions for implementing hook_forms().
   *
   * User per bundle form ids if possible, such that easy per bundle alterations
   * are supported too.
   */
  public function hook_forms() {
    $forms[$this->entityType . '_overview_form']['callback'] = 'entity_ui_overview_form';
    $forms[$this->entityType . '_operation_form']['callback'] = 'entity_ui_operation_form';

    // If there are bundles, use a form id including the bundle name to ease
    // per bundle alterations for the edit form.
    if (!(count($this->entityInfo['bundles']) == 1 && isset($this->entityInfo['bundles'][$this->entityType]))) {
      foreach ($this->entityInfo['bundles'] as $bundle => $bundle_info) {
        $forms[$this->entityType . '_edit_' . $bundle . '_form']['callback'] = $this->entityType . '_form';
      }
    }
    return $forms;
  }

  /**
   * Returns an entity overview form.
   */
  public function overviewForm($form, &$form_state) {
    // Test form element
    $form['filter'] = array(
      '#type' => 'fieldset',
      '#title' => t('Filter'),
      '#collapsible' => TRUE,
    );

    $entities = entity_load($this->entityType);

    $rows = array();
    foreach ($entities as $id => $entity) {
      $row = array();
      // Label
      $label = entity_label($this->entityType, $entity);
      if ($entity_uri = entity_uri($this->entityType, $entity)) {
        $row[] = l($label, url($entity_uri['path'], $entity_uri['options']));
      }
      else {
        $row[] = $label;
      }
      // Status (custom, default, overridden or fixed)
      // Operations (edit, clone, revert, delete)
      // @todo: This has to use entity_has_status().
      switch ($entity->status) {
        case ENTITY_IN_DB:
          $row[] = t('Custom');
          $row[] = l(t('edit'), $this->entityInfo['admin ui']['path'] . '/' . $id . '/edit');
          $row[] = l(t('clone'), $this->entityInfo['admin ui']['path'] . '/' . $id . '/clone');
          $row[] = l(t('delete'), $this->entityInfo['admin ui']['path'] . '/' . $id . '/delete', array('query' => drupal_get_destination()));
          break;
        case ENTITY_IN_CODE:
          $row[] = t('Default');
          $row[] = l(t('edit'), $this->entityInfo['admin ui']['path'] . '/' . $id . '/edit');
          $row[] = l(t('clone'), $this->entityInfo['admin ui']['path'] . '/' . $id . '/clone');
          break;
        case ENTITY_OVERRIDDEN:
          $row[] = t('Overridden');
          $row[] = l(t('edit'), $this->entityInfo['admin ui']['path'] . '/' . $id . '/edit');
          $row[] = l(t('clone'), $this->entityInfo['admin ui']['path'] . '/' . $id . '/clone');
          $row[] = l(t('revert'), $this->entityInfo['admin ui']['path'] . '/' . $id . '/revert', array('query' => drupal_get_destination()));
          break;
        case ENTITY_FIXED:
          $row[] = t('Fixed');
          $row[] = l(t('clone'), $this->entityInfo['admin ui']['path'] . '/' . $id . '/clone');
          break;
      }

      $rows[] = $row;
    }
    $header = array(t('Label'), t('Status'), array('data' => t('Operations'), 'colspan' => 4));

    $table = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('None'),
    );
    //$table['#attributes']['class'][] = 'rules-overview-table';
    //$table['#attached']['css'][] = drupal_get_path('module', 'rules') . '/ui/rules.ui.css';

    $form['list'] = $table;
    $form['list']['#caption'] = t('Available entities');
    $form['list']['#empty'] = t('There are no matching entities.');
    return $form;
  }

  public function overviewFormValidate() {}

  public function overviewFormSubmit() {}

  public function operationForm($form, &$form_state, $entity, $op) {
    $label = entity_label($this->entityType, $entity);
    $form_state += array(
      'type' => $this->entityType,
      'entity' => $entity,
      'op' => $op
    );
    switch ($op) {
      case 'revert':
        if (entity_has_status($this->entityType, $entity, ENTITY_OVERRIDDEN)) {
          $confirm_question = t('Are you sure you want to revert the %entity %label?', array('%entity' => $this->entityInfo['label'], '%label' => $label));
          return confirm_form($form, $confirm_question, $this->entityInfo['admin ui']['path']);
        }
        break;

      case 'delete':
        if (entity_has_status($this->entityType, $entity, ENTITY_IN_DB)) {
          $confirm_question = t('Are you sure you want to delete the %entity %label?', array('%entity' => $this->entityInfo['label'], '%label' => $label));
          return confirm_form($form, $confirm_question, $this->entityInfo['admin ui']['path']);
        }
        break;
    }
    drupal_not_found();
    exit;
  }

  public function operationFormSubmit($form, &$form_state) {
    if ($form_state['values']['confirm']) {
      $msg = $this->applyOperation($form_state['op'], $form_state['entity']);
      drupal_set_message($msg);
    }
  }

  /**
   * Applies an operation to the given entity.
   *
   * @param string $op
   *   The operation (edit, clone, revert or delete).
   * @param $entity
   *   The entity to manipulate.
   */
  public function operationApply($op, $entity) {
    $label = entity_label($this->entityType, $entity);
    $vars = array('%entity' => $this->entityInfo['label'], '%label' => $label);
    $edit_link = l(t('edit'), $this->entityInfo['admin ui']['path'] . '/' . $entity->id . '/edit');
    switch ($op) {
      case 'revert':
        $entity->delete();
        watchdog('entity', 'Reverted %entity %label to the defaults.', $vars, WATCHDOG_NOTICE, $edit_link);
        return t('Reverted %entity %label to the defaults.', $vars);

      case 'delete':
        $entity->delete();
        watchdog('entity', 'Deleted %entity %label.', $vars);
        return t('Deleted %entity %label.', $vars);

      default:
        return FALSE;
    }
  }

}

/**
 * Gets the edit form for an entity.
 *
 * This is an wrapper around drupal_get_form(), which is invoked with the right
 * form id being ENTITY_TYPE_form or, if the entity type has bundles
 * ENTITY_TYPE_edit_BUNDLE_form.
 *
 * @param $entity_type
 *   The entity type for which to get the form.
 * @param $entity
 *   The entity for whicht to return the form. If $op is 'add', NULL may be
 *   passed.
 * @param $op
 *   One of 'edit', 'add' or 'clone. Defaults to edit.
 *
 * @return
 *   The fully built and processed form, ready to be rendered.
 *
 * @see EntityDefaultUIController::hook_forms()
 */
function entity_ui_get_form($entity_type, $entity, $op = 'edit') {
  if (isset($entity)) {
    list(, , $bundle) = entity_extract_ids($entity_type, $entity);
  }
  $form_id = (!isset($bundle) || $bundle == $entity_type) ? $entity_type . '_form' : $entity_type . '_edit_' . $bundle . '_form';
  return drupal_get_form($form_id, $entity, $op);
}

/**
 * Form builder function for the overview form.
 *
 * Invokes the UI controller of the given entity type.
 */
function entity_ui_overview_form($form, &$form_state, $type) {
  return entity_ui_controller($type)->overviewForm($form, $form_state);
}

/**
 * Form builder for the entity operation form.
 *
 * Invokes the UI controller of the given entity type.
 */
function entity_ui_operation_form($form, &$form_state, $type, $entity, $op) {
  return entity_ui_controller($type)->operationForm($form, $form_state, $entity, $op);
}

/**
 * Entity operation form - submit callback.
 */
function entity_ui_operation_form_submit($form, &$form_state) {
  $type = $form_state['type'];
  entity_ui_controller($type)->operationFormSubmit($form, $form_state);
}

/**
 * Title callback for add operations on entities.
 *
 * @todo: currently unused.
 */
function entity_ui_add_title($type_label) {
  return t('Add @entity_type', array('@entity_type' => $type_label));
}

/**
 * Title callback for operations on entities.
 *
 * @todo: currently unused.
 */
function entity_ui_operation_title($op, $entity_type, $entity) {
  $label = entity_label($entity_type, $entity);
  switch ($op) {
    case 'edit':
      return t('Edit @entity', array('@entity' => $label));
    case 'clone':
      return t('Clone @entity', array('@entity' => $label));
    case 'revert':
      return t('Revert @entity', array('@entity' => $label));
    case 'delete':
      return t('Delete @entity', array('@entity' => $label));
  }
}
