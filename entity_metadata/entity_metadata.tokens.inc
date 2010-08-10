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
  $return = entity_metadata_token_types_chained();
  return $return + drupal_map_assoc(array('text', 'integer', 'decimal', 'duration', 'boolean', 'uri', 'site'));
}

/**
 * Defines a list of token types that need to be chained.
 *
 * @return
 *   If a type is given, whether the given type needs to be chained. Else a full
 *   list of token types to be chained as returned by
 *   entity_metadata_token_types().
 */
function entity_metadata_token_types_chained($type = NULL) {
  // Add entities.
  foreach (entity_get_info() as $entity_type => $info) {
    if ($token_type = isset($info['token type']) ? $info['token type'] : $entity_type) {
      $return[$token_type] = $entity_type;
    }
  }
  // Add 'date' tokens.
  $return['date'] = 'date';
  return isset($type) ? isset($return[$type]) : $return;
}

/**
 * Implements hook_token_info_alter().
 */
function entity_metadata_token_info_alter(&$info) {
  $valid_types = entity_metadata_token_types();
  $entity_info = entity_get_info();

  foreach ($valid_types as $token_type => $type) {
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
    // Make sure there is a token category for each supported entity.
    if (!empty($info['tokens'][$token_type]) && !isset($info['types'][$token_type]) && isset($entity_info[$type])) {
      $info['types'][$token_type] = array(
        'name' => $entity_info[$type]['label'],
        'description' => t('Tokens related to the %name entities.', array('%name' => $entity_info[$type]['label'])),
        'needs-data' => $token_type,
      );
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

    $info = token_info();
    foreach ($tokens as $name => $original) {
      // Provide the token for all properties marked to stem from us.
      if (!empty($info['tokens'][$type][$name]['entity metadata'])) {
        $wrapper = !isset($wrapper) ? _entity_metadata_token_wrap_data($type, $token_types[$type], $data[$type], $options) : $wrapper;
        $replacements[$original] = _entity_metadata_get_token($wrapper->$name, $options);
      }
    }
    // Properly chain everything of a type marked as needs chaining.
    foreach ($info['tokens'][$type] as $name => $token_info) {
      if (!empty($token_info['entity metadata']) && isset($token_info['type']) && entity_metadata_token_types_chained($token_info['type'])) {
        if ($chained_tokens = token_find_with_prefix($tokens, $name)) {
          $wrapper = !isset($wrapper) ? _entity_metadata_token_wrap_data($type, $token_types[$type], $data[$type], $options) : $wrapper;
          $replacements += token_generate($token_info['type'], $chained_tokens, array($token_info['type'] => $wrapper->$name->value()), $options);
        }
      }
    }
    return $replacements;
  }
}

function _entity_metadata_token_wrap_data($token_type, $type, $data, $options) {
  $wrapper = ($type == 'site') ? entity_metadata_site_wrapper() : entity_metadata_wrapper($type, $data);
  if (isset($options['language'])) {
    $wrapper->language($options['language']->language);
  }
  return $wrapper;
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
