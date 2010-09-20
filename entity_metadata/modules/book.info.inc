<?php
// $Id$

/**
 * @file
 * Provides info about book nodes.
 */

/**
 * Implements hook_entity_property_info_alter() on top of book module.
 * @see entity_metadata_entity_property_info_alter()
 */
function entity_metadata_book_entity_property_info_alter(&$info) {
  // Add meta-data about the added node properties.
  $properties = &$info['node']['properties'];

  $properties['book_id'] = array(
    'label' => t("Book ID"),
    'type' => 'integer',
    'validation callback' => 'entity_metadata_validate_integer_positive',
    'description' => t("If part of a book, the unique ID of this page's book."),
    'getter callback' => 'entity_metadata_book_get_properties',
  );
  $properties['book'] = array(
    'label' => t("Book"),
    'type' => 'node',
    'description' => t("If part of a book, the book to which this book page belongs."),
    'getter callback' => 'entity_metadata_book_get_properties',
  );
}