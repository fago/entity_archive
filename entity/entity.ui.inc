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
  protected $entityInfo, $path;

  public function __construct($entity_type, $entity_info) {
    $this->entityType = $entity_type;
    $this->entityInfo = $entity_info;
    $this->path = $this->entityInfo['admin ui']['path'];
    $this->statusKey = empty($this->entityInfo['entity keys']['status']) ? 'status' : $this->entityInfo['entity keys']['status'];
  }

  /**
   * Provides definitions for implementing hook_menu().
   */
  public function hook_menu() {
    $items = array();
    $id_count = count(explode('/', $this->path));

    $items[$this->path] = array(
      'title' => $this->entityInfo['label'] . 's',
      'page callback' => 'drupal_get_form',
      'page arguments' => array($this->entityType . '_overview_form', $this->entityType),
      'description' => 'Manage ' . $this->entityInfo['label'] . 's',
      'access callback' => 'entity_access',
      'access arguments' => array('view', $this->entityType),
      'file' => 'entity.ui.inc',
    );
    $items[$this->path . '/add'] = array(
      'title' => 'Add',
      'page callback' => 'entity_ui_get_form',
      'page arguments' => array($this->entityType, NULL, 'add'),
      'access callback' => 'entity_access',
      'access arguments' => array('create', $this->entityType),
      'type' => MENU_LOCAL_ACTION,
      'file' => 'entity.ui.inc',
    );
    $items[$this->path . '/%entity_menu/edit'] = array(
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
    $items[$this->path . '/%entity_menu/%'] = array(
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
    // The overview and the operation form are implemented by the controller,
    // the callback and validation + submit handlers just invoke the controller.
    $forms[$this->entityType . '_overview_form'] = array(
      'callback' => 'entity_ui_overview_form',
      'wrapper_callback' => 'entity_ui_form_defaults',
    );
    $forms[$this->entityType . '_operation_form'] = array(
      'callback' => 'entity_ui_operation_form',
      'wrapper_callback' => 'entity_ui_form_defaults',
    );

    // This is the main form for editing, adding and cloning.
    $forms[$this->entityType . '_form'] = array(
      'callback' => $this->entityType . '_form',
      'wrapper_callback' => 'entity_ui_form_defaults',
    );

    // If there are bundles, use a form id including the bundle name to ease
    // per bundle alterations for the edit form. In that case the general form
    // id needs to be defined too as done above, as it is invoked during 'add'.
    if (!(count($this->entityInfo['bundles']) == 1 && isset($this->entityInfo['bundles'][$this->entityType]))) {
      foreach ($this->entityInfo['bundles'] as $bundle => $bundle_info) {
        $forms[$this->entityType . '_edit_' . $bundle . '_form']['callback'] = $this->entityType . '_form';
      }
    }
    return $forms;
  }

  /**
   * Builds the entity overview form.
   */
  public function overviewForm($form, &$form_state) {
    // By default just show a simple overview for all entities.
    $form['table'] = $this->overviewTable();
    return $form;
  }

  /**
   * Overview form validation callback.
   */
  public function overviewFormValidate() {}

  /**
   * Overview form submit callback.
   */
  public function overviewFormSubmit() {}


  /**
   * Generates the render array for a overview table for arbitrary entities
   * matching the given conditions.
   *
   * @param $conditions
   *   An array of conditions as needed by entity_load().

   * @return Array
   *   A renderable array.
   */
  public function overviewTable($conditions = array()) {
    $entities = entity_load($this->entityType);

    $rows = array();
    foreach ($entities as $id => $entity) {
      $rows[] = $this->overviewTableRow($conditions, $id, $entity);
    }
    // Assemble the right table header.
    $header = array(t('Label'));
    if (!empty($this->entityInfo['exportable'])) {
      $header[] = t('Status');
    }
    $header[] = array('data' => t('Operations'), 'colspan' => 3);

    $render = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('None.'),
    );
    return $render;
  }

  /**
   * Generates the row for the passed entity and may be overridden in order to
   * customize the rows.
   *
   * @param $additional_cols
   *   Additional columns to be added after the entity label column.
   */
  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = array()) {
    // Optionally link to the entity if we have the URI.
    $label = entity_label($this->entityType, $entity);
    if ($entity_uri = entity_uri($this->entityType, $entity)) {
      $row[] = l($label, url($entity_uri['path'], $entity_uri['options']));
    }
    else {
      $row[] = $label;
    }

    // Add in any passed additional cols.
    foreach ($additional_cols as $col) {
      $row[] = $col;
    }

    // Add a row for the exportable status.
    if (!empty($this->entityInfo['exportable'])) {
      $row[] = array('data' => array(
        '#theme' => 'entity_status',
        '#status' => $entity->{$this->statusKey},
      ));
    }

    // Add operations depending on the status.
    if (entity_has_status($this->entityType, $entity, ENTITY_FIXED)) {
      $row[] = array('data' => l(t('clone'), $this->path . '/' . $id . '/clone'), 'colspan' => 3);
    }
    else {
      $row[] = l(t('edit'), $this->path . '/' . $id . '/edit');
      $row[] = l(t('clone'), $this->path . '/' . $id . '/clone');
      if (empty($this->entityInfo['exportable']) || !entity_has_status($this->entityType, $entity, ENTITY_IN_CODE)) {
        $row[] = l(t('delete'), $this->path . '/' . $id . '/delete', array('query' => drupal_get_destination()));
      }
      elseif (entity_has_status($this->entityType, $entity, ENTITY_OVERRIDDEN)) {
        $row[] = l(t('revert'), $this->path . '/' . $id . '/revert', array('query' => drupal_get_destination()));
      }
      else {
        $row[] = '';
      }
    }
    return $row;
  }


  /**
   * Builds the operation form.
   */
  public function operationForm($form, &$form_state, $entity, $op) {
    $label = entity_label($this->entityType, $entity);
    switch ($op) {
      case 'revert':
        $confirm_question = t('Are you sure you want to revert the %entity %label?', array('%entity' => $this->entityInfo['label'], '%label' => $label));
        return confirm_form($form, $confirm_question, $this->path);

      case 'delete':
        $confirm_question = t('Are you sure you want to delete the %entity %label?', array('%entity' => $this->entityInfo['label'], '%label' => $label));
        return confirm_form($form, $confirm_question, $this->path);
    }
    drupal_not_found();
    exit;
  }

  /**
   * Operation form validation callback.
   */
  public function operationFormValidate($form, &$form_state) { }

  /**
   * Operation form submit callback.
   */
  public function operationFormSubmit($form, &$form_state) {
    $msg = $this->applyOperation($form_state['op'], $form_state[$this->entityType]);
    drupal_set_message($msg);
  }

  /**
   * Applies an operation to the given entity.
   *
   * @param string $op
   *   The operation (edit, clone, revert or delete).
   * @param $entity
   *   The entity to manipulate.
   *
   * @return
   *   The status message of what has been applied.
   */
  public function applyOperation($op, $entity) {
    $label = entity_label($this->entityType, $entity);
    $vars = array('%entity' => $this->entityInfo['label'], '%label' => $label);
    $id = entity_id($this->entityType, $entity);
    $edit_link = l(t('edit'), $this->path . '/' . $id . '/edit');

    switch ($op) {
      case 'revert':
        entity_delete($this->entityType, $id);
        watchdog($this->entityType, 'Reverted %entity %label to the defaults.', $vars, WATCHDOG_NOTICE, $edit_link);
        return t('Reverted %entity %label to the defaults.', $vars);

      case 'delete':
        entity_delete($this->entityType, $id);
        watchdog($this->entityType, 'Deleted %entity %label.', $vars);
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
 * @see EntityDefaultUIController::overviewForm()
 */
function entity_ui_overview_form($form, &$form_state, $entity_type) {
  return entity_ui_controller($entity_type)->overviewForm($form, $form_state);
}

/**
 * Form builder for the entity operation form.
 *
 * @see EntityDefaultUIController::operationForm()
 */
function entity_ui_operation_form($form, &$form_state, $entity_type, $entity, $op) {
  $form_state['op'] = $op;
  return entity_ui_controller($entity_type)->operationForm($form, $form_state, $entity, $op);
}

/**
 * Form wrapper callback for all entity ui forms.
 *
 * This callback makes sure the form state is properly initialized and sets
 * some useful default titles.
 *
 * @see EntityDefaultUIController::hook_forms()
 */
function entity_ui_form_defaults($form, &$form_state, $entity_type, $entity = NULL, $op = NULL) {
  $defaults = array(
    'entity_type' => $entity_type,
  );
  if (isset($entity)) {
    $defaults[$entity_type] = $entity;
  }
  if (isset($op)) {
    $defaults['op'] = $op;
  }
  $form_state += $defaults;
  if (isset($op)) {
    drupal_set_title(entity_ui_get_page_title($op, $entity_type, $entity));
  }
  // Add in handlers pointing to the controller for forms implemented by it.
  if ($form_state['build_info']['base_form_id'] != $entity_type . '_form') {
    $form['#validate'][] = 'entity_ui_controller_form_validate';
    $form['#submit'][] = 'entity_ui_controller_form_submit';
  }
  return $form;
}

/**
 * Validation callback for forms implemented by the UI controller.
 */
function entity_ui_controller_form_validate($form, &$form_state) {
  // Remove 'entity_ui_' prefix and the '_form' suffix.
  $base = substr($form_state['build_info']['base_form_id'], 10, -5);
  $method = $base . 'FormValidate';
  entity_ui_controller($form_state['entity_type'])->$method($form, $form_state);
}

/**
 * Submit callback for forms implemented by the UI controller.
 */
function entity_ui_controller_form_submit($form, &$form_state) {
  // Remove 'entity_ui_' prefix and the '_form' suffix.
  $base = substr($form_state['build_info']['base_form_id'], 10, -5);
  $method = $base . 'FormSubmit';
  entity_ui_controller($form_state['entity_type'])->$method($form, $form_state);
}

/**
 * Gets the page title for the passed operation.
 */
function entity_ui_get_page_title($op, $entity_type, $entity = NULL) {
  if ($op != 'add') {
    $label = entity_label($entity_type, $entity);
  }
  switch ($op) {
    case 'add':
      $info = entity_get_info($entity_type);
      return t('Add @entity_type', array('@entity_type' => $info['label']));
    case 'edit':
      return t('Edit @label', array('@label' => $label));
    case 'clone':
      return t('Clone @label', array('@label' => $label));
    case 'revert':
      return t('Revert @label', array('@label' => $label));
    case 'delete':
      return t('Delete @label', array('@label' => $label));
  }
}
