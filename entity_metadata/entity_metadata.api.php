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
 *     property. Known keys are:
 *     - label: A human readable, translated label for the property.
 *     - description: A human readable, translated description for the property.
 *     - type: The data type of the property. To make the property actually
 *       useful it's important to map your properties to one of the known data
 *       types, which currently are:
 *        - text
 *        - integer
 *        - decimal
 *        - date: As timestamp.
 *        - duration: In number of seconds.
 *        - boolean
 *        - uri: Be sure to always return absolute URIs.
 *        - entities - You may use the type of each entity known by
 *          hook_entity_info(), e.g. 'node' or 'user'.
 *       Also lists of these types are supported. Specify list<TYPE> as type and
 *       return an numerically indexed array of values.
 *     - sanitized: For textual properties only, whether the text is already
 *       sanitized. In this case you might want to also specify a raw getter
 *       callback. Defaults to FALSE.
 *     - sanitize: For textual properties, that aren't sanitized yet, specify
 *       a function for sanitizing the value. Defaults to check_plain().
 *     - 'getter callback': A callback used to retrieve the value of the
 *       property. Defaults to entity_metadata_verbatim_get().
 *     - 'setter callback': A callback used to set the value of the property.
 *       This is optional, however entity_metadata_verbatim_set() can be used.
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
 *   - bundles: An array keyed by bundle name containing further metadata
 *     related to the bundles only. This array may contain the key 'properties'
 *     with an array of info about the bundle specific properties, structured in
 *     the same way as the entity properties array.
 *
 *  @see hook_entity_metadata_info_alter()
 *  @see entity_metadata_get_info()
 *  @see entity_metadata_wrapper()
 */
function hook_entity_metadata_info() {
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
 * @see hook_entity_metadata_info()
 */
function hook_entity_metadata_info_alter(&$info) {
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
function entity_metadata_field_info() {
  return array(
    'text' => array(
      'label' => t('Text'),
      'property_type' => 'text',
      // ...
    ),
  );
}

/**
 * @} End of "addtogroup hooks".
 */
