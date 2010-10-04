<?php
// $Id$

/**
 * @file
 * Provides a base class for entities.
 */

/**
 * A common class for db entities.
 *
 * It is suggested to also implement the label() and uri() methods for the
 * respective callbacks in hook_entity_info(). For that case the provided
 * callbacks entity_class_label() and entity_class_uri() may be specified.
 */
class EntityDB {

  protected $entityType;
  protected $entityInfo;
  protected $idKey, $nameKey;

  public function __construct(array $values = array(), $entityType = NULL) {
    if (empty($entityType)) {
      throw new Exception('Cannot created an instance of EntityDB without a specified entity type.');
    }
    $this->entityType = $entityType;
    $this->entityInfo = entity_get_info($entityType);
    $this->idKey = $this->entityInfo['entity keys']['id'];
    $this->nameKey = isset($this->entityInfo['entity keys']['name']) ? $this->entityInfo['entity keys']['name'] : $this->idKey;

    // Set initial values.
    foreach ($values as $key => $value) {
      $this->$key = $value;
    }
  }

  public function internalIdentifier() {
    return isset($this->{$this->idKey}) ? $this->{$this->idKey} : NULL;
  }

  public function identifier() {
    return isset($this->{$this->nameKey}) ? $this->{$this->nameKey} : NULL;
  }

  public function entityInfo() {
    return $this->entityInfo;
  }

  public function entityType() {
    return $this->entityType;
  }

  public function save() {
    return entity_get_controller($this->entityType)->save($this);
  }

  public function delete() {
    $id = $this->identifier();
    if (isset($id)) {
      entity_get_controller($this->entityType)->delete(array($id));
    }
  }

  public function export($prefix = '') {
    return entity_get_controller($this->entityType)->export($this, $prefix);
  }

  public function view($view_mode = 'full', $langcode = NULL) {
    return entity_get_controller($this->entityType)->view(array($this->identifier() => $this), $view_mode, $langcode);
  }

  public function buildContent($view_mode = 'full', $langcode = NULL) {
    return entity_get_controller($this->entityType)->buildContent($this, $view_mode, $langcode);
  }
}
