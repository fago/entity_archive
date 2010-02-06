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
    'query callback' => 'entity_metadata_table_query',
  );
  $properties['vid'] = array(
    'label' => t("Revision ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the node's revision."),
    'query callback' => 'entity_metadata_table_query',
  );
  $properties['tnid'] = array(
    'label' => t("Translation set ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the original-language version of this node, if one exists."),
    'query callback' => 'entity_metadata_table_query',
  );
  $properties['uid'] = array(
    'label' => t("User ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the author of the node."),
    'setter callback' => 'entity_metadata_verbatim_set',
    'setter permission' => 'administer nodes',
    'clear' => array('author'),
    'query callback' => 'entity_metadata_table_query',
  );
  $properties['is-new'] = array(
    'label' => t("Is new"),
    'type' => 'boolean',
    'description' => t("Whether the node is new and not saved to the database yet."),
    'getter callback' => 'entity_metadata_node_get_properties',
  );
  $properties['type'] = array(
    'label' => t("Content type"),
    'description' => t("The type of the node."),
    'setter callback' => 'entity_metadata_verbatim_set',
    'setter permission' => 'administer nodes',
    'options list' => 'node_type_get_names',
    'required' => TRUE,
    'query callback' => 'entity_metadata_table_query',
  );
  $properties['type-name'] = array(
    'label' => t("Content type name"),
    'description' => t("The human-readable name of the node type."),
    'getter callback' => 'entity_metadata_node_get_properties',
  );
  $properties['title'] = array(
    'label' => t("Title"),
    'description' => t("The title of the node."),
    'setter callback' => 'entity_metadata_verbatim_set',
    'query callback' => 'entity_metadata_table_query',
    'required' => TRUE,
  );
  $properties['language'] = array(
    'label' => t("Language"),
    'description' => t("The language the node is written in."),
    'setter callback' => 'entity_metadata_verbatim_set',
    'options list' => 'entity_metadata_language_list',
    'query callback' => 'entity_metadata_table_query',
    'setter permission' => 'administer nodes',
  );
  $properties['url'] = array(
    'label' => t("URL"),
    'description' => t("The URL of the node."),
    'getter callback' => 'entity_metadata_entity_get_properties',
    'type' => 'uri',
  );
  $properties['edit-url'] = array(
    'label' => t("Edit URL"),
    'description' => t("The URL of the node's edit page."),
    'getter callback' => 'entity_metadata_node_get_properties',
    'type' => 'uri',
  );
  $properties['status'] = array(
    'label' => t("Published"),
    'description' => t("Whether the node is published."),
    'setter callback' => 'entity_metadata_verbatim_set',
    'setter permission' => 'administer nodes',
    'query callback' => 'entity_metadata_table_query',
    'type' => 'boolean',
  );
  $properties['promote'] = array(
    'label' => t("Promoted to frontpage"),
    'description' => t("Whether the node is promoted to the frontpage."),
    'setter callback' => 'entity_metadata_verbatim_set',
    'setter permission' => 'administer nodes',
    'query callback' => 'entity_metadata_table_query',
    'type' => 'boolean',
  );
  $properties['sticky'] = array(
    'label' => t("Sticky in lists"),
    'description' => t("Whether the node is displayed at the top of lists in which it appears."),
    'setter callback' => 'entity_metadata_verbatim_set',
    'setter permission' => 'administer nodes',
    'query callback' => 'entity_metadata_table_query',
    'type' => 'boolean',
  );
  $properties['created'] = array(
    'label' => t("Date created"),
    'type' => 'date',
    'description' => t("The date the node was posted."),
    'setter callback' => 'entity_metadata_verbatim_set',
    'setter permission' => 'administer nodes',
    'query callback' => 'entity_metadata_table_query',
  );
  $properties['changed'] = array(
    'label' => t("Date changed"),
    'type' => 'date',
    'query callback' => 'entity_metadata_table_query',
    'description' => t("The date the node was most recently updated."),
  );
  $properties['author'] = array(
    'label' => t("Author"),
    'type' => 'user',
    'description' => t("The author of the node."),
    'getter callback' => 'entity_metadata_node_get_properties',
    'setter callback' => 'entity_metadata_node_set_properties',
    'setter permission' => 'administer nodes',
    'required' => TRUE,
    'clear' => array('uid'),
  );
  return $info;
}

