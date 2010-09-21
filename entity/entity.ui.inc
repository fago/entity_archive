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
  }

  public function formValidate() {}

  public function formSubmit() {}

}

function entity_overview_form($form, &$form_state, $type) {
  // Test form element
  $form['filter'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter'),
    '#collapsible' => TRUE,
  );
  return $form;
}
