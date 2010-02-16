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
    $return = entity_uri($entity_type, $entity);
    return url($return['path'], $return['options'] + $options);
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
      return $node->book['bid'];
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

    case 'edit-url':
      return url('comment/edit/' . $comment->cid, $options);

    case 'node':
      return node_load($comment->nid);

    case 'parent':
      if (!empty($comment->pid)) {
        return $comment->pid;
      }
      throw new EntityMetadataWrapperException('This comment has no parent comment.');

    case 'author':
      return $comment->uid;
  }
}

/**
 * Callback for setting comment properties.
 * @see entity_metadata_comment_entity_info_alter()
 */
function entity_metadata_comment_setter($comment, $name, $value) {
  switch ($name) {
    case 'node':
      $comment->nid = $value;
      // Also set the bundle name.
      $node = node_load($value);
      $comment->node_type = 'comment_node_' . $node->type;
      break;

    case 'author':
      return $comment->uid = $value;
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
      return $node->uid;
  }
}

/**
 * Callback for setting node properties.
 * @see entity_metadata_node_entity_info_alter()
 */
function entity_metadata_node_set_properties($node, $name, $value) {
  if ($name == 'author') {
    $node->uid = $value;
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
      return $GLOBALS['user']->uid;

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
      return $file->uid;
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
      return $term->vid;

    case 'parent':
      if (isset($term->parent[0]) && !is_array(isset($term->parent[0]))) {
        return $term->parent;
      }
      return array_keys(taxonomy_get_parents($term->tid));
  }
}

/**
 * Callback for setting term properties.
 * @see entity_metadata_taxonomy_entity_info_alter()
 */
function entity_metadata_taxonomy_term_setter($term, $name, $value) {
  switch ($name) {
    case 'vocabulary':
      return $term->vid = $value;
    case 'parent':
      return $term->parent = $value;
  }
}

/**
 * Callback for getting vocabulary properties.
 * @see entity_metadata_taxonomy_entity_info_alter()
 */
function entity_metadata_taxonomy_vocabulary_get_properties($vocabulary, array $options, $name) {
  switch ($name) {
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

    case 'roles':
      return isset($account->roles) ? array_keys($account->roles) : array();
  }
}

/**
 * Callback for setting user properties.
 * @see entity_metadata_user_entity_info_alter()
 */
function entity_metadata_user_set_properties($account, $name, $value) {
  switch ($name) {
    case 'roles':
      $account->roles = array_intersect_key(user_roles(), array_flip($value));
      break;
  }
}


/**
 * Options list callback for user roles.
 */
function entity_metadata_user_roles() {
  return user_roles(TRUE);
}


/**
 * Callback defining an options list for language properties.
 */
function entity_metadata_language_list() {
  $list = array();
  $list[LANGUAGE_NONE] = t('Language neutral');
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
  $field = field_info_field($name);
  $key = reset(array_keys($field['columns']));
  $langcode = _entity_metadata_field_property_get_langcode($object, $options, $name);
  $values = array();
  if (isset($object->{$name}[$langcode])) {
    foreach ($object->{$name}[$langcode] as $delta => $data) {
      $values[$delta] = $data[$key];
    }
  }
  return $field['cardinality'] == 1 ? reset($values) : $values;
}

function _entity_metadata_field_property_get_langcode($object, $options, $name) {
  $langcode = LANGUAGE_NONE;
  if (isset($options['language']) && isset($object->{$name}[$options['language']->language])) {
    $langcode = $options['language']->language;
  }
  return $langcode;
}

/**
 * Callback for setting field property values.
 */
function entity_metadata_field_property_set($object, $name, $value, $langcode, $obj_type) {
  $name = str_replace('-', '_', $name);
  $field = field_info_field($name);
  $key = reset(array_keys($field['columns']));
  $langcode = isset($langcode) ? $langcode : LANGUAGE_NONE;
  $values = $field['cardinality'] == 1 ? array($value) : $value;

  $items = array();
  foreach ($values as $delta => $value) {
    $items[$delta][$key] = $value;
  }
  $object->{$name}[$langcode] = $items;
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
 * Callback to verbatim get the data structure of a field. Useful for fields
 * that add metadata for their own data structure.
 *
 * It's important to return a reference on the field's item structure, so any
 * changes to the data through the wrapper are present in the object too.
 */
function &entity_metadata_field_verbatim_get($object, array $options, $name, $obj_type, &$context) {
  $name = str_replace('-', '_', $name);
  // Set contextual info useful for getters of any child properties.
  $entity_info = $context['parent']->info();
  $context['instance'] = field_info_instance($entity_info['type'], $name, $entity_info['bundle']);
  $context['field'] = field_info_field($name);

  $langcode = _entity_metadata_field_property_get_langcode($object, $options, $name);
  if (!isset($object->{$name}[$langcode])) {
    $object->{$name}[$langcode] = array(0 => array());
  }
  if ($context['field']['cardinality'] == 1) {
    return $object->{$name}[$langcode][0];
  }
  return $object->{$name}[$langcode];
}

/**
 * Writes the passed field items in the object. Useful as field level setter for
 * to set the whole data structure at once.
 */
function entity_metadata_field_verbatim_set($object, $name, $items, $langcode) {
  $name = str_replace('-', '_', $name);
  $field = field_info_field($name);
  $langcode = isset($langcode) ? $langcode : LANGUAGE_NONE;
  $object->{$name}[$langcode] = $field['cardinality'] == 1 ? array($items) : $items;
}

/**
 * Callback for getting the sanitized text of processed text fields.
 */
function entity_metadata_field_text_get(&$item, array $options, $name, $type, $context) {
  if (!isset($item['safe_value'])) {
    // Get the the wrapper of the field's property.
    $field_wrapper = _entity_metadata_field_get_wrapper($context);
    $info = $field_wrapper->info();
    $item += array('format' => filter_default_format());
    $langcode = isset($options['language']) ? $options['language']->language : '';
    $item['safe_value'] = _text_sanitize($info['instance'], $langcode, $item, 'value');
  }
  return $item['safe_value'];
}

/**
 * Get the wrapper of a field given a context of one of the field's properties.
 */
function _entity_metadata_field_get_wrapper($context) {
  // If this is a multiple value field, there is list wrapper in between.
  if ($context['parent'] instanceof EntityListWrapper && $info = $context['parent']->info()) {
    return $info['parent'];
  }
  return $context['parent'];
}

/**
 * Defines the list of formats available for the current user.
 */
function entity_metadata_field_text_formats() {
  foreach (filter_formats($GLOBALS['user']) as $key => $format) {
    $formats[$key] = $format->name;
  }
  return $formats;
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
  if (user_access('bypass node access', $account) || (!isset($account) && $op == 'view' && node_access_view_all_nodes())) {
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

/**
 * Callback to create entity objects.
 */
function entity_metadata_create_object($values = array(), $entity_type) {
  $info = entity_get_info($entity_type);
  $class = isset($info['entity class']) ? $info['entity class'] : 'stdClass';
  $object = new $class(array(), $entity_type);
  // Support specifying the id of the entity for some entity types.
  if (isset($info['object keys']['id']) && ($id_key = $info['object keys']['id']) && isset($values[$id_key])) {
    $object->$id_key = $values[$id_key];
    $object->is_new = TRUE;
    unset($values[$id_key]);
  }
  // Make sure at least the bundle property is set.
  if (isset($info['object keys']['bundle']) && $bundle_key = $info['object keys']['bundle']) {
    $object->$bundle_key = isset($values[$bundle_key]) ? $values[$bundle_key] : NULL;
  }
  $wrapper = entity_metadata_wrapper($entity_type, $object);
  foreach ($values as $name => $value) {
    $wrapper->$name->set($value);
  }
  return $wrapper;
}

/**
 * Callback to create a new comment.
 */
function entity_metadata_create_comment($values = array()) {
  $comment = (object)array(
    'status' => COMMENT_PUBLISHED,
    'cid' => FALSE,
    'pid' => 0,
    'language' => LANGUAGE_NONE,
    'node_type' => NULL,
  );
  $wrapper = entity_metadata_wrapper('comment', $comment);
  foreach ($values as $name => $value) {
    $wrapper->$name->set($value);
  }
  return $wrapper;
}

/**
 * Callback to create a new node.
 */
function entity_metadata_create_node($values = array()) {
  $node = (object)array(
    'type' => $values['type'],
    'language' => LANGUAGE_NONE,
    'is_new' => TRUE,
    // Support specifying custom node ids.
    'nid' => isset($values['nid']) ? $values['nid'] : FALSE,
  );
  unset($values['nid']);
  // Set some defaults
  $node_options = variable_get('node_options_' . $node->type, array('status', 'promote'));
  foreach (array('status', 'promote', 'sticky') as $key) {
    $node->$key = (int) in_array($key, $node_options);
  }
  $wrapper = entity_metadata_wrapper('node', $node);
  foreach ($values as $name => $value) {
    $wrapper->$name->set($value);
  }
  return $wrapper;
}

/**
 * Callback to save a user account. Unfortunately drupal doesn't provide a
 * simple save function for user accounts, one has to specify the fields to
 * save explictly. So we automatically do so for field API and regular user
 * fields.
 */
function entity_metadata_user_save(&$account) {
  $table = drupal_get_schema('users');
  $user_fields = $table['fields'];
  list($id, $vid, $bundle) = entity_extract_ids('user', $account);
  $instances = field_info_instances('user', $bundle);
  // Extract the values of all valid keys into $edit.
  $edit = array_intersect_key((array)$account, $user_fields + $instances);
  $account = user_save($account, $edit);
}

/**
 * Callback to delete a file.
 */
function entity_metadata_file_delete($fid) {
  file_delete(file_load($fid), TRUE);
}

/**
 * Callback for querying entity properties having their values stored in the
 * entities main db table.
 */
function entity_metadata_table_query($entity_type, $property, $value, $limit) {
  $info = entity_get_info($entity_type);
  $name_key = isset($info['object keys']['name']) ? $info['object keys']['name'] : $info['object keys']['id'];
  $query = db_select($info['base table'], 'base')
             ->fields('base', array($name_key))
             ->condition($property, $value)
             ->range(0, $limit);
  return $query->execute()->fetchCol();
}

/**
 * Callback for querying entities by field values. Only suitable for fields
 * that don't process the data, so it's stored the same way as returned.
 */
function entity_metadata_field_query($entity_type, $property, $value, $limit) {
  $field = field_info_field(str_replace('-', '_', $property));
  $key = reset(array_keys($field['columns']));
  $conditions[] = array('type', $entity_type);
  $conditions[] = array($key, $value);
  $result = field_attach_query($field['id'], $conditions, array('limit' => $limit));
  return array_keys($result[$entity_type]);
}
