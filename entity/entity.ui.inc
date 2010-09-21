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
   * Provides a menu item as expected by hook_menu().
   */
  public function menu() {
    return array(
      'title' => $this->entity_info['label'],
      'page callback' => 'drupal_get_form',
      'page arguments' => array('entity_overview_form', $this->entity_type),
      'description' => 'Administer entity type ' . $this->entity_info['label'],
      'access callback' => 'entity_access',
      'access arguments' => array('view', $this->entity_type),
      'file' => 'entity.ui.inc',
    );
  }

  /**
   * Returns an entity overview form.
   */
  public function form() {
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
      $row[] = t('edit');
      $row[] = t('clone');
      $row[] = t('delete');

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

  public function formValidate() {}

  public function formSubmit() {}

}

function entity_overview_form($form, &$form_state, $type) {
  return entity_ui_controller($type)->form();
}
