<?php
// $Id$

/**
 * @file
 * Provides various callbacks for the whole core module integration.
 */

/**
 * Callback for getting properties of an entity.
 */
function entity_metadata_entity_get_properties($entity, array $options, $name, $entity_type) {
  if ($name == 'url') {
    return url(entity_path($entity_type, $entity), $options);
  }
}

/**
 * Callback for getting book node properties.
 * @see entity_metadata_book_entity_info_alter()
 */
function entity_metadata_book_get_properties($node, array $options, $name, $entity_type) {
  if (!isset($node->book['bid'])) {
    throw new EntityMetadataWrapperException('This node is no book page.');
  }
  switch ($name) {
    case 'book-id':
      return $node->book['bid'];

    case 'book':
      return node_load($node->book['bid']);
  }
}

/**
 * Callback for getting comment properties.
 * @see entity_metadata_comment_entity_info_alter()
 */
function entity_metadata_comment_get_properties($comment, array $options, $name) {
  switch ($name) {
    case 'name':
      return $name = ($comment->uid == 0) ? variable_get('anonymous', t('Anonymous')) : $comment->name;

    case 'mail':
      if ($comment->uid != 0) {
        $account = user_load($comment->uid);
        return $account->mail;
      }
      return $comment->mail;

    case 'title':
      return $comment->subject;

    case 'edit-url':
      return url('comment/edit/' . $comment->cid, $options);

    case 'node':
      return node_load($comment->nid);

    case 'parent':
      if ($parent = comment_load($comment->pid)) {
        return $parent;
      }
      throw new EntityMetadataWrapperException('This comment has no parent comment.');

    case 'author':
      return user_load($comment->uid);
  }
}

/**
 * Callback for getting raw comment properties.
 * @see entity_metadata_comment_entity_info_alter()
 */
function entity_metadata_comment_get_raw($comment, array $options, $name) {
  switch ($name) {
    case 'body':
      return $comment->comment;
  }
}

/**
 * Callback for getting comment related node properties.
 * @see entity_metadata_comment_entity_info_alter()
 */
function entity_metadata_comment_get_node_properties($node, array $options, $name, $entity_type) {
  switch ($name) {
    case 'comment-count':
      return isset($node->comment_count) ? $node->comment_count : 0;

    case 'comment-count-new':
      return comment_num_new($node->nid);
  }
}

/**
 * Callback for getting node properties.
 * @see entity_metadata_node_entity_info_alter()
 */
function entity_metadata_node_get_properties($node, array $options, $name, $entity_type) {
  switch ($name) {
    case 'is-new':
      return empty($node->nid) || !empty($node->is_new);

    case 'type-name':
      return node_type_get_name($node->type);

    case 'edit-url':
      return url('node/' . $node->nid . '/edit', $options);

    case 'author':
      return user_load($node->uid);
  }
}

/**
 * Callback for setting node properties.
 * @see entity_metadata_node_entity_info_alter()
 */
function entity_metadata_node_set_properties($node, $name, $value) {
  if ($name == 'author') {
    $node->uid = $value->uid;
    $node->name = $value->name;
  }
}

/**
 * Callback for getting poll properties.
 * @see entity_metadata_poll_entity_info_alter()
 */
function entity_metadata_poll_node_get_properties($node, array $options, $name) {
  $total_votes = $highest_votes = 0;
  foreach ($node->choice as $choice) {
    if ($choice['chvotes'] > $highest_votes) {
      $winner = $choice;
      $highest_votes = $choice['chvotes'];
    }
    $total_votes = $total_votes + $choice['chvotes'];
  }

  if ($name == 'poll-duration') {
    return $node->runtime;
  }
  elseif ($name == 'poll-votes') {
    return $total_votes;
  }
  elseif (!isset($winner)) {
    throw new EntityMetadataWrapperException('There is no poll winner yet.');
  }
  switch ($name) {
    case 'poll-winner-votes':
        return $winner['chvotes'];

    case 'poll-winner':
        return $winner['chtext'];

    case 'poll-winner-percent':
        return ($winner['chvotes'] / $total_votes) * 100;
  }
}

/**
 * Callback for getting statistics properties.
 * @see entity_metadata_statistics_entity_info_alter()
 */
function entity_metadata_statistics_node_get_properties($node, array $options, $name) {
  $statistics = statistics_get($node->nid);

  switch ($name) {
    case 'views':
      return $statistics['totalviews'];

    case 'day-views':
      return $statistics['dayviews'];

    case 'last-view':
      return $statistics['timestamp'];
  }
}

/**
 * Callback for getting system properties.
 * @see entity_metadata_system_entity_info_alter()
 */
function entity_metadata_system_get_properties($data = FALSE, array $options, $name) {
  switch ($name) {
    case 'name':
      return variable_get('site_name', 'Drupal');

    case 'url':
      return url('<front>', $options);

    case 'login-url':
      return url('user', $options);

    case 'current-user':
      return user_load($GLOBALS['user']->uid);

    default:
      return variable_get('site_' . $name, '');
  }
}

/**
 * Callback for getting file properties.
 * @see entity_metadata_system_entity_info_alter()
 */
function entity_metadata_system_get_file_properties($file, array $options, $name) {
  switch ($name) {
    case 'name':
      return $file->filename;

    case 'mime':
      return $file->filemime;

    case 'size':
      return $file->filesize;

    case 'url':
      return url(file_create_url($file->uri), $options);

    case 'owner':
      return user_load($file->uid);
  }
}

/**
 * Callback for getting term properties.
 * @see entity_metadata_taxonomy_entity_info_alter()
 */
function entity_metadata_taxonomy_term_get_properties($term, array $options, $name) {
  switch ($name) {
    case 'node-count':
      return count(taxonomy_select_nodes($term->tid));

    case 'description':
      return check_markup($term->description, $term->format, '', TRUE);

    case 'vocabulary':
      return taxonomy_vocabulary_load($term->vid);

    case 'parent':
      if (($parents = taxonomy_get_parents($term->tid)) && $parent = array_pop($parents)) {
        return taxonomy_term_load($parent->tid);
      }
      throw new EntityMetadataWrapperException('This term has no parent term.');
  }
}

/**
 * Callback for getting vocabulary properties.
 * @see entity_metadata_taxonomy_entity_info_alter()
 */
function entity_metadata_taxonomy_vocabulary_get_properties($vocabulary, array $options, $name) {
  switch ($name) {
    case 'node-count':
      $sql = "SELECT COUNT (1) FROM {taxonomy_term_node} tn LEFT JOIN {taxonomy_term_data} td ON tn.tid = td.tid WHERE td.vid = :vid";
      return db_query($sql, array(':vid' => $vocabulary->vid))->fetchField();

    case 'term-count':
      $sql = "SELECT COUNT (1) FROM {taxonomy_term_data} td WHERE td.vid = :vid";
      return db_query($sql, array(':vid' => $vocabulary->vid))->fetchField();
  }
}

/**
 * Callback for getting user properties.
 * @see entity_metadata_user_entity_info_alter()
 */
function entity_metadata_user_get_properties($account, array $options, $name, $entity_type) {
  switch ($name) {
    case 'name':
      return ($account->uid == 0) ? variable_get('anonymous', t('Anonymous')) : $account->name;

    case 'edit-url':
      return url("user/$account->uid/edit", $options);
  }
}

/**
 * Callback defining an options list for language properties.
 */
function entity_metadata_language_list() {
  $list = array();
  foreach (language_list() as $language) {
    $list[$language->language] = t($language->name);
  }
  return $list;
}

/**
 * Callback for getting field property values.
 */
function entity_metadata_field_property_get($object, array $options, $name, $obj_type) {
  $name = str_replace('-', '_', $name);
  $langcode = _entity_metadata_field_property_get_langcode($object, $options, $name);
  return $object->{$name}[$langcode][0]['value'];
}

/**
 * Callback for getting multiple field property values.
 */
function entity_metadata_field_property_get_multiple($object, array $options, $name, $obj_type) {
  $name = str_replace('-', '_', $name);
  $langcode = _entity_metadata_field_property_get_langcode($object, $options, $name);
  $values = array();
  if (isset($object->{$name}[$langcode])) {
    foreach ($object->{$name}[$langcode] as $delta => $data) {
      $values[$delta] = $data['value'];
    }
  }
  return $values;
}

/**
 * Callback for getting the 'safe_value' of a field.
 */
function entity_metadata_field_property_get_safe($object, array $options, $name, $obj_type) {
  $name = str_replace('-', '_', $name);
  $langcode = _entity_metadata_field_property_get_langcode($object, $options, $name);
  entity_metadata_field_refresh_sanitized_value($object, $obj_type, $name, $langcode);
  return $object->{$name}[$langcode][0]['safe_value'];
}

/**
 * Callback for getting multiple 'safe_value' values of a field.
 */
function entity_metadata_field_property_get_multiple_safe($object, array $options, $name, $obj_type) {
  $name = str_replace('-', '_', $name);
  $langcode = _entity_metadata_field_property_get_langcode($object, $options, $name);
  entity_metadata_field_refresh_sanitized_value($object, $obj_type, $name, $langcode);
  $values = array();
  if (isset($object->{$name}[$langcode])) {
    foreach ($object->{$name}[$langcode] as $delta => $data) {
      $values[$delta] = $data['safe_value'];
    }
  }
  return $values;
}

function _entity_metadata_field_property_get_langcode($object, $options, $name) {
  $langcode = LANGUAGE_NONE;
  if (isset($options['language']) && isset($object->{$name}[$options['language']->language])) {
    $langcode = $options['language']->language;
  }
  return $langcode;
}

/**
 * Makes sure the sanitized 'safe_value' is present.
 */
function entity_metadata_field_refresh_sanitized_value($object, $obj_type, $field_name, $langcode) {
  if (!isset($object->{$field_name}[$langcode][0]['safe_value'])) {
    $field = field_info_field($field_name);
    list($id, $vid, $bundle) = entity_extract_ids($obj_type, $object);
    $instance = field_info_instance($obj_type, $field_name, $bundle);

    // TODO: Currently this doesn't work for text* field types.
    // _field_invoke('sanitize', $obj_type, $object, $null, $null, array('language' => $langcode));
    // This makes it work for text* field types only for now.
    foreach ($object->{$field_name}[$langcode] as $key => $item) {
      $object->{$field_name}[$langcode][$key]['safe_value'] = _text_sanitize($instance, $langcode, $item, 'value');
    }
  }
}

/**
 * Callback for setting field property values.
 */
function entity_metadata_field_property_set($object, $name, $value, $langcode, $obj_type) {
  $name = str_replace('-', '_', $name);
  $langcode = isset($langcode) ? $langcode : LANGUAGE_NONE;
  $object->{$name}[$langcode][0]['value'] = $value;
  unset($object->{$name}[$langcode][0]['safe_value']);
}

/**
 * Callback for setting multiple field property values.
 */
function entity_metadata_field_property_set_multiple($object, $name, $values, $langcode, $obj_type) {
  $name = str_replace('-', '_', $name);
  $langcode = isset($langcode) ? $langcode : LANGUAGE_NONE;
  $object->{$name}[$langcode] = array();
  foreach ($values as $key => $value) {
    $object->{$name}[$langcode][$key]['value'] = $value;
  }
}

/**
 * Callback returning the options list of a field.
 */
function entity_metadata_field_options_list($object, $name, $obj_type) {
  $name = str_replace('-', '_', $name);
  $field = field_info_field($name);
  return (array)module_invoke($field['module'], 'options_list', $field);
}

/**
 * Access callback for the node entity.
 */
function entity_metadata_node_access($op, $entity_type, $node = NULL, $account = NULL) {
  if (isset($node)) {
    return node_access($op, $node, $account);
  }
  // Is access to all nodes allowed?
  if (!user_access('access content', $account)) {
    return FALSE;
  }
  if (user_access('bypass node access', $account) || (!isset($account) && node_access_view_all_nodes())) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Access callback for the user entity.
 */
function entity_metadata_user_access($op, $entity_type, $entity = NULL, $account = NULL) {
  $account = isset($account) ? $account : $GLOBALS['user'];
  // Grant access to the users own user account and to the anonymous one.
  if (isset($entity) && $op != 'delete' && (($entity->uid == $account->uid && $entity->uid) || (!$entity->uid && $op == 'view'))) {
    return TRUE;
  }
  if (user_access('administer users', $account) || user_access('access user profiles', $account) && $op == 'view') {
    return TRUE;
  }
  return FALSE;
}

/**
 * Access callback for the comment entity.
 */
function entity_metadata_comment_access($op, $entity_type, $entity = NULL, $account = NULL) {
  if (isset($entity) && !isset($account) && comment_access($op, $entity)) {
    return TRUE;
  }
  if (user_access('administer comments', $account) || user_access('access comments', $account) && $op == 'view') {
    return TRUE;
  }
  return FALSE;
}

/**
 * Access callback for the taxonomy entities.
 */
function entity_metadata_taxonomy_access($op, $entity_type, $entity = NULL, $account = NULL) {
  if ($entity_type == 'taxonomy_vocabulary') {
    return user_access('administer taxonomy', $account);
  }
  if (isset($entity) && $op == 'edit' && !isset($account) && taxonomy_term_edit_access($entity)) {
    return TRUE;
  }
  if (user_access('administer taxonomy', $account) || user_access('access content', $account) && $op == 'view') {
    return TRUE;
  }
  return FALSE;
}

/**
 * Callback to determine access for properties which are fields.
 */
function entity_metadata_field_access_callback($op, $name, $entity_type, $entity = NULL, $account = NULL) {
  $field_name = str_replace('-', '_', $name);
  $field = field_info_field($field_name);
  return field_access($op, $field, $entity_type, $entity, $account);
}
