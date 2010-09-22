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
      // menu path for edit, clone, delete
      $path = $this->entity_info['admin ui']['path'] . '/%entity_menu/%';
      $id_count = count(explode('/', $path)) - 2;
      $op_count = $id_count + 1;
      $items[$path] = array(
        'page callback' => 'drupal_get_form',
        'page arguments' => array('entity_confirm_op', $this->entity_type, $id_count, $op_count),
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

    foreach ($entities as $id => $entity) {
      $row = array();
      $label = entity_label($this->entity_type, $entity);
      if ($entity_uri = entity_uri($this->entity_type, $entity)) {
        $row[] = l($label, url($entity_uri['path'], $entity_uri['options']));
      }
      else {
        $row[] = $label;
      }
      $row[] = l(t('edit'), $this->entity_info['admin ui']['path'] . '/' . $id . '/edit');
      $row[] = l(t('clone'), $this->entity_info['admin ui']['path'] . '/' . $id . '/clone');
      $row[] = l(t('delete'), $this->entity_info['admin ui']['path'] . '/' . $id . '/delete', array('query' => drupal_get_destination()));

      $rows[] = $row;
    }
    $header = array(t('Label'), array('data' => t('Operations'), 'colspan' => 3));

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
    switch ($op) {
      case 'revert':
        $confirm_question = t('Are you sure you want to revert the %entity %label?', array('%entity' => $this->entity_info['label'], '%label' => $label));
        $confirm_form = confirm_form($form, $confirm_question, $this->entity_info['admin ui']['path']);
        break;
      case 'delete':
        $confirm_question = t('Are you sure you want to delete the %entity %label?', array('%entity' => $this->entity_info['label'], '%label' => $label));
        $confirm_form = confirm_form($form, $confirm_question, $this->entity_info['admin ui']['path']);
        break;
      default:
        drupal_not_found();
        exit;
    }
    $form_state += array(
      'type' => $this->entity_type,
      'entity' => $entity,
      'op' => $op
    );
    return $confirm_form;
  }

  public function operationFormSubmit($form, &$form_state) {
    if ($form_state['values']['confirm']) {
      $entity = $form_state['entity'];
      $label = entity_label($this->entity_type, $entity);
      $vars = array('%entity' => $this->entity_info['label'], '%label' => $label);

      switch ($form_state['op']) {
        case 'revert':
          $entity->delete();
          watchdog('entity', 'Reverted %entity %label to the defaults.', $vars);
          drupal_set_message(t('Reverted %entity %label to the defaults.', $vars));
          break;
        case 'delete':
          $entity->delete();
          watchdog('entity', 'Deleted %entity %label.', $vars);
          drupal_set_message(t('Deleted %entity %label.', $vars));
          break;
      }
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
 * Entity operation confirm form.
 */
function entity_confirm_op($form, &$form_state, $type, $entity, $op) {
  return entity_ui_controller($type)->operationForm($form, $form_state, $entity, $op);
}

/**
 * Entity operation confirm form - submit callback.
 */
function entity_confirm_op_submit($form, &$form_state) {
  $type = $form_state['type'];
  entity_ui_controller($type)->operationFormSubmit($form, $form_state);
}
