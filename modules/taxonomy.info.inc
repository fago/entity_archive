<?php
// $Id$

/**
 * @file
 * Provides info about the taxonomy entity.
 */

/**
 * Implements hook_entity_property_info() on top of taxonomy module.
 * @see entity_entity_property_info()
 */
function entity_metadata_taxonomy_entity_property_info() {
  $info = array();
  // Add meta-data about the basic taxonomy properties.
  $properties = &$info['taxonomy_term']['properties'];

  $properties['tid'] = array(
    'label' => t("Term ID"),
    'description' => t("The unique ID of the taxonomy term."),
    'type' => 'integer',
  );
  $properties['name'] = array(
    'label' => t("Name"),
    'description' => t("The name of the taxonomy term."),
    'setter callback' => 'entity_property_verbatim_set',
    'required' => TRUE,
  );
  $properties['description'] = array(
    'label' => t("Description"),
    'description' => t("The optional description of the taxonomy term."),
    'sanitized' => TRUE,
    'raw getter callback' => 'entity_property_verbatim_get',
    'getter callback' => 'entity_metadata_taxonomy_term_get_properties',
    'setter callback' => 'entity_property_verbatim_set',
  );
  $properties['weight'] = array(
    'label' => t("Weight"),
    'type' => 'integer',
    'description' => t('The weight of the term, which is used for ordering terms during display.'),
    'setter callback' => 'entity_property_verbatim_set',
  );
  $properties['node_count'] = array(
    'label' => t("Node count"),
    'type' => 'integer',
    'description' => t("The number of nodes tagged with the taxonomy term."),
    'getter callback' => 'entity_metadata_taxonomy_term_get_properties',
  );
  $properties['url'] = array(
    'label' => t("URL"),
    'description' => t("The URL of the taxonomy term."),
    'getter callback' => 'entity_metadata_entity_get_properties',
    'type' => 'uri',
  );
  $properties['vocabulary'] = array(
    'label' => t("Vocabulary"),
    'description' => t("The vocabulary the taxonomy term belongs to."),
    'getter callback' => 'entity_metadata_taxonomy_term_get_properties',
    'setter callback' => 'entity_metadata_taxonomy_term_setter',
    'type' => 'taxonomy_vocabulary',
    'required' => TRUE,
  );
  $properties['parent'] = array(
    'label' => t("Parent terms"),
    'description' => t("The parent terms of the taxonomy term."),
    'getter callback' => 'entity_metadata_taxonomy_term_get_properties',
    'setter callback' => 'entity_metadata_taxonomy_term_setter',
    'type' => 'list<taxonomy_term>',
  );

  // Add meta-data about the basic vocabulary properties.
  $properties = &$info['taxonomy_vocabulary']['properties'];

  // Taxonomy vocabulary related variables.
  $properties['vid'] = array(
    'label' => t("Vocabulary ID"),
    'description' => t("The unique ID of the taxonomy vocabulary."),
    'type' => 'integer',
  );
  $properties['name'] = array(
    'label' => t("Name"),
    'description' => t("The name of the taxonomy vocabulary."),
    'setter callback' => 'entity_property_verbatim_set',
    'required' => TRUE,
  );
  $properties['machine_name'] = array(
    'label' => t("Machine name"),
    'type' => 'token',
    'description' => t("The machine name of the taxonomy vocabulary."),
    'setter callback' => 'entity_property_verbatim_set',
    'required' => TRUE,
  );
  $properties['description'] = array(
    'label' => t("Description"),
    'description' => t("The optional description of the taxonomy vocabulary."),
    'setter callback' => 'entity_property_verbatim_set',
    'sanitize' => 'filter_xss',
  );
  $properties['term_count'] = array(
    'label' => t("Term count"),
    'type' => 'integer',
    'description' => t("The number of terms belonging to the taxonomy vocabulary."),
    'getter callback' => 'entity_metadata_taxonomy_vocabulary_get_properties',
  );
  return $info;
}
