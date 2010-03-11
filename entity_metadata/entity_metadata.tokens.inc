<?php
// $Id$

/**
 * @file
 * Provides tokens for entity properties which have no token yet.
 */

/**
 * Defines the types of properties to be added as token.
 *
 * @return
 *   An array mapping token types to the usual (entity) type names.
 */
function entity_metadata_token_types() {
  foreach (entity_get_info() as $type => $info) {
    if ($token_type = isset($info['token type']) ? $info['token type'] : $type) {
      $return[$token_type] = $type;
    }
  }
  return drupal_map_assoc(array('text', 'integer', 'decimal', 'date', 'duration', 'boolean', 'uri')) + $return;
}

/**
 * Implements hook_token_info_alter().
 */
function entity_metadata_token_info_alter(&$info) {
  $valid_types = entity_metadata_token_types();

  foreach (entity_get_info() as $type => $entity_info) {
    $token_type = isset($entity_info['token type']) ? $entity_info['token type'] : $type;
    if (!$token_type) {
      continue;
    }

    if (!isset($info['types'][$token_type])) {
      $info['types']['user'] = array(
        'name' => $entity_info['label'],
        'description' => t('Tokens related to the %name entities.', array('%name' => $entity_info['label'])),
        'needs-data' => $token_type,
      );
    }
    // Just add all properties regardless whether its in a bundle only if there
    // is no token of the property yet.
    foreach (entity_metadata_get_properties($type) as $name => $property) {
      if (!isset($info['tokens'][$token_type][$name]) && (!isset($property['type']) || in_array($property['type'], $valid_types))) {
        $info['tokens'][$token_type][$name] = array(
          'name' => $property['label'],
          'type' => isset($property['type']) ? array_search($property['type'], $valid_types) : 'text',
          // Mark the token so we know we have to provide the value afterwards.
          'entity metadata' => TRUE,
        );
        $info['tokens'][$token_type][$name] += array_intersect_key($property, array_flip(array('description')));
      }
    }
  }
}

/**
 * Implements hook_tokens().
 */
function entity_metadata_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $token_types = entity_metadata_token_types();
  if (isset($token_types[$type]) && (!empty($data[$type]) || $type == 'site')) {
    $data += array($type => FALSE);
    $replacements = array();
    if (isset($options['language'])) {
      $wrapper->language($options['language']->language);
    }

    $info = token_info();
    foreach ($tokens as $name => $original) {
      // Provide the token for all properties marked to stem from us.
      if (!empty($info['tokens'][$type][$name]['entity metadata'])) {
        $wrapper = !isset($wrapper) ? entity_metadata_wrapper($token_types[$type], $data[$type]) : $wrapper;
        $replacements[$original] = _entity_metadata_get_token($wrapper->$name, $options);
      }
    }
    // Properly chain all date tokens that are marked to stem from us.
    foreach ($info['tokens'][$type] as $name => $token_info) {
      if (!empty($token_info['entity metadata']) && isset($token_info['type']) && $token_info['type'] == 'date') {
        if ($date_tokens = token_find_with_prefix($tokens, $name)) {
          $wrapper = !isset($wrapper) ? entity_metadata_wrapper($token_types[$type], $data[$type]) : $wrapper;
          $replacements += token_generate('date', $date_tokens, array('date' => $wrapper->$name->value()), $options);
        }
      }
    }
  }
  return $replacements;
}

function _entity_metadata_get_token($wrapper, $options) {
  if (empty($options['sanitize'])) {
    // When we don't need sanitized tokens decode already sanitizied texts.
    $options['decode'] = TRUE;
  }
  $langcode = isset($options['language']) ? $options['language']->language : NULL;

  // If there are options use them to format the token.
  if ($options_list = $wrapper->optionsList()) {
    // Flatten the options array.
    foreach ($options_list as $entry) {
      if (is_array($entry)) {
        $options_list += $entry;
      }
    }
    return empty($options['sanitize']) ? $options_list[$wrapper->value()] : filter_xss($options_list[$wrapper->value()]);
  }

  switch ($wrapper->type()) {
    case 'integer':
      return number_format($wrapper->value());
    case 'decimal':
      return number_format($wrapper->value(), 2);
    case 'date':
      return format_date($wrapper->value(), 'medium', '', NULL, $langcode);
    case 'duration':
      return format_interval($wrapper->value(), 2, $langcode);
    case 'boolean':
      return $wrapper->value() ? t('true') : t('false');
    case 'uri':
    case 'text':
      return $wrapper->value($options);
  }
}
