<?php
// $Id$

/**
 * @file
 * Provides info about the node entity.
 */

/**
 * Implements hook_entity_property_info() on top of node module.
 * @see entity_metadata_entity_property_info()
 */
function entity_metadata_node_entity_property_info() {
  $info = array();
  // Add meta-data about the basic node properties.
  $properties = &$info['node']['properties'];

  $properties['nid'] = array(
    'label' => t("Node ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the node."),
  );
  $properties['vid'] = array(
    'label' => t("Revision ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the node's revision."),
  );
  $properties['tnid'] = array(
    'label' => t("Translation set ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the original-language version of this node, if one exists."),
  );
  $properties['uid'] = array(
    'label' => t("User ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the author of the node."),
    'setter callback' => 'entity_metadata_verbatim_set',
    'clear' => array('author'),
  );
  $properties['type'] = array(
    'label' => t("Content type"),
    'description' => t("The type of the node."),
  );
  $properties['type-name'] = array(
    'label' => t("Content type name"),
    'description' => t("The human-readable name of the node type."),
    'getter callback' => 'entity_metadata_node_get_properties',
  );
  $properties['language'] = array(
    'label' => t("Language"),
    'description' => t("The language the node is written in."),
    'setter callback' => 'entity_metadata_verbatim_set',
  );
  $properties['url'] = array(
    'label' => t("URL"),
    'description' => t("The URL of the node."),
    'getter callback' => 'entity_metadata_node_get_properties',
    'type' => 'uri',
  );
  $properties['edit-url'] = array(
    'label' => t("Edit URL"),
    'description' => t("The URL of the node's edit page."),
    'getter callback' => 'entity_metadata_node_get_properties',
    'type' => 'uri',
  );
  $properties['created'] = array(
    'label' => t("Date created"),
    'type' => 'date',
    'description' => t("The date the node was posted."),
    'setter callback' => 'entity_metadata_verbatim_set',
  );
  $properties['changed'] = array(
    'label' => t("Date changed"),
    'type' => 'date',
    'description' => t("The date the node was most recently updated."),
  );
  $properties['author'] = array(
    'label' => t("Author"),
    'type' => 'user',
    'description' => t("The author of the node."),
    'getter callback' => 'entity_metadata_node_get_properties',
  );
  return $info;
}

