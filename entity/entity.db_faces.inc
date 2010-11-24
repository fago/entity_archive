<?php
// $Id$

/**
 * @file
 * Provides an extendable base class for entities based upon faces.
 */

/**
 * A class for entities, which is extendable via the extendable object faces API.
 *
 * If there is no specific reason to use the faces API, it is suggested to
 * make use of the class Entity instead.
 *
 * @see Entity
 */
class EntityExtendable extends FacesExtendable {

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

  public function __sleep() {
    // By default serialize everything but the faces class instances.
    $all = get_object_vars($this);
    unset($all['facesClassInstances']);
    return array_keys($all);
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

/**
 * This class is deprecated by "EntityExtendable" and only here for backward compatibility reasons.
 */
class EntityDBExtendable extends EntityExtendable {

}
