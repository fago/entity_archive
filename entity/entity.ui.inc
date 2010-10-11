<?php
// $Id$

/**
 * @file
 * Provides a controller for building an entity overview form.
 */

class EntityDefaultUIController {

  protected $entity_type;

  protected $entity_info;

  public function __construct($entity_type, $entity_info) {
    $this->entity_type = $entity_type;
    $this->entity_info = $entity_info;
  }

  /**
   * Provides menu items as expected by hook_menu().
   */
  public function menu() {
    $items = array();
    $items[$this->entity_info['admin ui']['path']] = array(
      'title' => $this->entity_info['label'],
      'page callback' => 'drupal_get_form',
      'page arguments' => array('entity_overview_form', $this->entity_type),
      'description' => 'Administer entity type ' . $this->entity_info['label'],
      'access callback' => 'entity_access',
      'access arguments' => array('view', $this->entity_type),
      'file' => 'entity.ui.inc',
    );
    $entities = entity_load($this->entity_type);
    foreach ($entities as $id => $entity) {
      // Edit form
      $path = $this->entity_info['admin ui']['path'] . '/%entity_menu/edit';
      $id_count = count(explode('/', $path)) - 2;
      $items[$path] = array(
        'page callback' => 'drupal_get_form',
        'page arguments' => array($this->entity_type . '_form', $this->entity_type, $id_count, 'edit'),
        'load arguments' => array($this->entity_type),
        'access callback' => 'entity_access',
        'access arguments' => array('update', $this->entity_type),
        'type' => MENU_CALLBACK,
      );
      // Change form id if there is a bundle
      list(,,$bundle) = entity_extract_ids($this->entity_type, $entity);
      if ($bundle != $this->entity_type) {
        $itmes[$path]['page arguments'] = array($this->entity_type . '_edit_' . $bundle . '_form', $this->entity_type, $id_count, 'edit');
      }
      // Clone form, a special case for the edit form.
      $path = $this->entity_info['admin ui']['path'] . '/%entity_menu/clone';
      $items[$path] = array(
        'page callback' => 'drupal_get_form',
        'page arguments' => array($this->entity_type . '_form', $this->entity_type, $id_count, 'clone'),
        'load arguments' => array($this->entity_type),
        'access callback' => 'entity_access',
        'access arguments' => array('update', $this->entity_type),
        'type' => MENU_CALLBACK,
      );
      // Change form id if there is a bundle
      if ($bundle != $this->entity_type) {
        $itmes[$path]['page arguments'] = array($this->entity_type . '_edit_' . $bundle . '_form', $this->entity_type, $id_count, 'clone');
      }
      // menu path for operations like revert, delete
      $path = $this->entity_info['admin ui']['path'] . '/%entity_menu/%';
      $op_count = $id_count + 1;
      $items[$path] = array(
        'page callback' => 'drupal_get_form',
        'page arguments' => array($this->entity_type . '_operation_form', $this->entity_type, $id_count, $op_count),
        'load arguments' => array($this->entity_type),
        'access callback' => 'entity_access',
        'access arguments' => array($op_count, $this->entity_type),
        'type' => MENU_CALLBACK,
        'file' => 'entity.ui.inc',
      );
    }
    return $items;
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

    $entities = entity_load($this->entity_type);

    $rows = array();
    foreach ($entities as $id => $entity) {
      $row = array();
      // Label
      $label = entity_label($this->entity_type, $entity);
      if ($entity_uri = entity_uri($this->entity_type, $entity)) {
        $row[] = l($label, url($entity_uri['path'], $entity_uri['options']));
      }
      else {
        $row[] = $label;
      }
      // Status (custom, default, overridden or fixed)
      // Operations (edit, clone, revert, delete)
      switch ($entity->status) {
        case ENTITY_IN_DB:
          $row[] = t('Custom');
          $row[] = l(t('edit'), $this->entity_info['admin ui']['path'] . '/' . $id . '/edit');
          $row[] = l(t('clone'), $this->entity_info['admin ui']['path'] . '/' . $id . '/clone');
          $row[] = l(t('delete'), $this->entity_info['admin ui']['path'] . '/' . $id . '/delete', array('query' => drupal_get_destination()));
          break;
        case ENTITY_IN_CODE:
          $row[] = t('Default');
          $row[] = l(t('edit'), $this->entity_info['admin ui']['path'] . '/' . $id . '/edit');
          $row[] = l(t('clone'), $this->entity_info['admin ui']['path'] . '/' . $id . '/clone');
          break;
        case ENTITY_OVERRIDDEN:
          $row[] = t('Overridden');
          $row[] = l(t('edit'), $this->entity_info['admin ui']['path'] . '/' . $id . '/edit');
          $row[] = l(t('clone'), $this->entity_info['admin ui']['path'] . '/' . $id . '/clone');
          $row[] = l(t('revert'), $this->entity_info['admin ui']['path'] . '/' . $id . '/revert', array('query' => drupal_get_destination()));
          break;
        case ENTITY_FIXED:
          $row[] = t('Fixed');
          $row[] = l(t('clone'), $this->entity_info['admin ui']['path'] . '/' . $id . '/clone');
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
    $label = entity_label($this->entity_type, $entity);
    $form_state += array(
      'type' => $this->entity_type,
      'entity' => $entity,
      'op' => $op
    );
    switch ($op) {
      case 'revert':
        if (entity_has_status($this->entity_type, $entity, ENTITY_OVERRIDDEN)) {
          $confirm_question = t('Are you sure you want to revert the %entity %label?', array('%entity' => $this->entity_info['label'], '%label' => $label));
          return confirm_form($form, $confirm_question, $this->entity_info['admin ui']['path']);
        }
        break;

      case 'delete':
        if (entity_has_status($this->entity_type, $entity, ENTITY_IN_DB)) {
          $confirm_question = t('Are you sure you want to delete the %entity %label?', array('%entity' => $this->entity_info['label'], '%label' => $label));
          return confirm_form($form, $confirm_question, $this->entity_info['admin ui']['path']);
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
    $label = entity_label($this->entity_type, $entity);
    $vars = array('%entity' => $this->entity_info['label'], '%label' => $label);
    $edit_link = l(t('edit'), $this->entity_info['admin ui']['path'] . '/' . $entity->id . '/edit');
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
 * Form API callback that invokes the UI controller of the given type to render
 * the entity overview form.
 */
function entity_overview_form($form, &$form_state, $type) {
  return entity_ui_controller($type)->overviewForm($form, $form_state);
}

/**
 * Entity operation form.
 */
function entity_operation_form($form, &$form_state, $type, $entity, $op) {
  return entity_ui_controller($type)->operationForm($form, $form_state, $entity, $op);
}

/**
 * Entity operation form - submit callback.
 */
function entity_operation_form_submit($form, &$form_state) {
  $type = $form_state['type'];
  entity_ui_controller($type)->operationFormSubmit($form, $form_state);
}