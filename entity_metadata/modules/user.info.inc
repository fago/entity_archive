<?php
// $Id$

/**
 * @file
 * Provides info about the user entity.
 */

/**
 * Implements hook_entity_property_info() on top of user module.
 * @see entity_metadata_entity_property_info()
 */
function entity_metadata_user_entity_property_info() {
  $info = array();
  // Add meta-data about the user properties.
  $properties = &$info['user']['properties'];

  $properties['uid'] = array(
    'label' => t("User ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the user account."),
  );
  $properties['name'] = array(
    'label' => t("Name"),
    'description' => t("The login name of the user account."),
    'getter callback' => 'entity_metadata_user_get_properties',
    'setter callback' => 'entity_metadata_verbatim_set',
    'sanitize' => 'filter_xss',
  );
  $properties['mail'] = array(
    'label' => t("Email"),
    'description' => t("The email address of the user account."),
    'setter callback' => 'entity_metadata_verbatim_set',
  );
  $properties['url'] = array(
    'label' => t("URL"),
    'description' => t("The URL of the account profile page."),
    'getter callback' => 'entity_metadata_entity_get_properties',
    'type' => 'uri',
  );
  $properties['edit-url'] = array(
    'label' => t("Edit URL"),
    'description' => t("The url of the account edit page."),
    'getter callback' => 'entity_metadata_user_get_properties',
    'type' => 'uri',
  );
  $properties['login'] = array(
    'label' => t("Last login"),
    'description' => t("The date the user last logged in to the site."),
    'type' => 'date',
  );
  $properties['created'] = array(
    'label' => t("Created"),
    'description' => t("The date the user account was created."),
    'type' => 'date',
  );
  return $info;
}

