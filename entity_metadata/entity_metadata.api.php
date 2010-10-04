<?php
// $Id$

/**
 * @file
 * Hooks provided by the entity metadata module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Allow modules to define metadata about entity properties.
 *
 * Modules providing properties for any entities defined in hook_entity_info()
 * can implement this hook to provide metadata about this properties. This is
 * separated from hook_entity_info() for performance reasons only.
 * For making use of the metadata have a look at the provided wrappers returned
 * by entity_metadata_wrapper().
 * For providing entity metadata for fields see entity_metadata_field_info().
 *
 * @return
 *   An array whose keys are entity type names and whose values are arrays
 *   containing the keys:
 *   - properties: The array describing all properties for this entity. Entries
 *     are keyed by the property name and contain an array of metadata for each
 *     property. The name may only contain alphanumeric lowercase characters
 *     and underscores. Known keys are:
 *     - label: A human readable, translated label for the property.
 *     - description: A human readable, translated description for the property.
 *     - type: The data type of the property. To make the property actually
 *       useful it's important to map your properties to one of the known data
 *       types, which currently are:
 *        - text: Any text.
 *        - token: A string containing only lowercase letters, numbers, and
 *          underscores starting with a letter; e.g. this type is useful for
 *          machine readable names.
 *        - integer: A usual PHP integer value.
 *        - decimal: A PHP float or integer.
 *        - date: A full date and time, as timestamp.
 *        - duration: A duration as number of seconds.
 *        - boolean: A usual PHP boolean value.
 *        - uri: An absolute URI or URL.
 *        - entities - You may use the type of each entity known by
 *          hook_entity_info(), e.g. 'node' or 'user'. Internally entities are
 *          represented by their identifieres.
 *        - struct: This as well as any else not known type may be used for
 *          supporting arbitrary data structures. For that additional metadata
 *          has to be specified with the 'property info' key.
 *        - list: A list of values, represented as numerically indexed array.
 *          The list<TYPE> notation may be used to specify the type of the
 *          contained items, where TYPE may be any valid type expression.
 *     - sanitized: For textual properties only, whether the text is already
 *       sanitized. In this case you might want to also specify a raw getter
 *       callback. Defaults to FALSE.
 *     - sanitize: For textual properties, that aren't sanitized yet, specify
 *       a function for sanitizing the value. Defaults to check_plain().
 *     - 'getter callback': A callback used to retrieve the value of the
 *       property. Defaults to entity_metadata_verbatim_get().
 *     - 'setter callback': A callback used to set the value of the property.
 *       This is optional, however entity_metadata_verbatim_set() can be used.
 *     - 'validation callback': An optional callback that returns whether the
 *       passed data value is valid for the property. May be used to implement
 *       additional checks, such as to ensure the value is a valid mail address.
 *     - clear: An array of property names, of which the cache should be cleared
 *       too once this property is updated. E.g. the author uid property wants
 *       to have the author property cleared too. Optional.
 *     - 'raw getter callback': For sanitized textual properties, a separate
 *       callback which can be used to retrieve the raw, unprocessed value.
 *     - bundle: If the property is an entity, you may specify the bundle of the
 *       retrieved entity. Optional.
 *     - 'options list': Optionally, a callback that returns a list of key value
 *       pairs for the property. The callback has to return an array as
 *       used by hook_options_list().
 *     - 'access callback': An optional access callback to allow for checking
 *       'view' and 'edit' access for the described property. If no callback
 *       is specified, a 'setter permission' may be specified instead.
 *     - 'setter permission': Optionally a permission, that describes whether
 *       a user has permission to set ('edit') this property. This permission
 *       should only be taken into account, if no 'access callback' is given.
 *     - 'query callback: Optionally a callback for querying for entities
 *       having the given property value. See entity_metadata_entity_query().
 *     - required: Optionally, this may be set to TRUE, if this property is
 *       required for the creation of a new instance of its entity. See
 *       entity_metadata_entity_create().
 *     - field: Optionally, a boolean indicating whether a property is stemming
 *       from a field.
 *     - 'property info': Optionally, may be used to specify an array of info
 *       for an arbitrary data structure together with any else not defined
 *       type. Specify metadata in the same way as used by this hook.
 *     - 'property info alter': Optionally, a callback for altering the property
 *       info before it is used.
 *     - 'property defaults': Optionally, an array of defaults for the info of
 *       each property of the wrapped data item.
 *   - bundles: An array keyed by bundle name containing further metadata
 *     related to the bundles only. This array may contain the key 'properties'
 *     with an array of info about the bundle specific properties, structured in
 *     the same way as the entity properties array.
 *
 *  @see hook_entity_property_info_alter()
 *  @see entity_metadata_get_info()
 *  @see entity_metadata_wrapper()
 */
function hook_entity_property_info() {
  $info = array();
  $properties = &$info['node']['properties'];

  $properties['nid'] = array(
    'label' => t("Node ID"),
    'type' => 'integer',
    'description' => t("The unique ID of the node."),
  );
  return $info;
}

/**
 * Allow modules to alter metadata about entity properties.
 *
 * @see hook_entity_property_info()
 */
function hook_entity_property_info_alter(&$info) {
  $properties = &$info['node']['bundles']['poll']['properties'];

  $properties['poll-votes'] = array(
    'label' => t("Poll votes"),
    'description' => t("The number of votes that have been cast on a poll node."),
    'type' => 'integer',
    'getter callback' => 'entity_metadata_poll_node_get_properties',
  );
}

/**
 * Provide metadata for fields.
 *
 * For providing entity metadata for fields each field type may specify a
 * property type to map to in its field info using the key 'property_type'. With
 * that info in place default property info is generated, which is already
 * suiting for a lot of field types. However it's possible to specify further
 * callbacks, that may alter the generated property info. To specify those use
 * the key 'property_callbacks' and set it to an array of function names.
 * Apart from that any property info provided for a field instance using the
 * key 'property info' is added in too.
 *
 * @see entity_metadata_field_info_alter()
 * @see entity_metadata_field_text_property_callback()
 */
function entity_metadata_hook_field_info() {
  return array(
    'text' => array(
      'label' => t('Text'),
      'property_type' => 'text',
      // ...
    ),
  );
}

/**
 * Provide additional metadata for entities.
 *
 * This defines further keys to annotate more metadata in hook_entity_info().
 * Additional keys are:
 * - access callback: Specify a callback that returns access permissions for the
 *   operations 'create', 'updated', 'delete' and 'view'. The callback gets
 *   optionally the entity and the user account to check for passed. See
 *   entity_metadata_no_hook_node_access() for an example.
 * - creation callback: Optionally, a callback that creates a new instance of
 *   this entity type. See entity_metadata_create_node() for an example.
 * - save callback: Optionally, a callback that permanently saves an entity of
 *   this type.
 * - deletion callback: Optionally, a callback that permanently deletes an
 *   entity of this type.
 * - token type: Optionally a type name to use for token replacements. Set it
 *   to FALSE if there aren't any token replacements for this entity type.
 *
 * @see hook_entity_info()
 * @see entity_metadata_entity_access()
 * @see entity_metadata_entity_create()
 * @see entity_metadata_entity_save()
 * @see entity_metadata_entity_delete()
 */
function entity_metadata_hook_entity_info() {
  return array(
    'node' => array(
      'label' => t('Node'),
      'access callback' => 'entity_metadata_no_hook_node_access',
      // ...
    ),
  );
}

/**
 * @} End of "addtogroup hooks".
 */
