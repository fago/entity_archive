$Id$

Entity APIs
------------
by Wolfgang Ziegler, nuppla@zites.net

This are API modules. You only need to enable them, if a module depends on one
of them or you are interested in using them for development.


This README is for interested developers. If you are not interested in developing,
you may stop reading now.


--------------------------------------------------------------------------------
                              Entity CRUD API
--------------------------------------------------------------------------------

 * To use the API you have to introduce a dependency on the "entity" module.

 * The provided controller implements full CRUD functionality, which may be used
   via the provided helpers entity_create(), entity_save(), entity_delete().
   
   Alternatively you may specify a class to use for your entities, for which the
   "Entity" class is provided. In particular, it is useful to extend this class
   in order to apply any necessary customizations.
   
 * The controller supports fieldable entities as well as exportable entities,
   however it does not yet support revisions.
   
 * The Entity CRUD API helps with providing addition module integration too,
   e.g. exportable entities are automatically integrate with the Features
   module. These module integrations are implemented in separate controller
   classes, which may be overridden and deactivated on their own.
   
 * There is also an optional ui controller class, which assits with providing an
   administrative UI for managing entities of a certain type.
   
 * For more details check out the documentation in the drupal.org handbook:
   http://drupal.org/node/878804
 

 How to add a new entity type?
 ------------------------------
 
  * You might want to study the code of the "entity_test.module".
  
  * Describe your entities db table as usual in hook_schema().
  
  * You may use the entity classes directly or extend it with your own class.
    To see how to provide a separate class have a look at the "EntityClass" from
    the "entity_test.module". 
  
  * Implement hook_entity_info() for your entity. At least specifiy the
    controller class of this API, your db table and your object's primary key
    field. Optionally also set the 'entity class' to Entity or your extended
    class.
    Again just look at "entity_test.module"'s hook_entity_info() for guidance.
    
  * If you want your entity to be fieldable just set 'fieldable' in
    hook_entity_info() to TRUE. The field API attachers are called automatically
    in the entity CRUD functions then.
    
  * The entity API is able to deal with bundle objects too (e.g. the node type
    object). For that just specify an entity type for the bundle objects and
    set the 'bundle of' property appropriate.
    Again just look at "entity_test.module"'s hook_entity_info() for guidance.

  * Schema fields marked as 'serialzed' are automatically unserialized upon
    loading as well as serialized on saving. If the 'merge' attribute is also
    set to TRUE the unserialized data is automatically "merged" into the entity.

  * Further details can be found at http://drupal.org/node/878804.    
    


--------------------------------------------------------------------------------
                              Entity Metadata
--------------------------------------------------------------------------------

  * This module introduces a unique place for metadata about entities and their
    properties. For that hook_entity_info() already used by core is extended,
    and hook_entity_property_info() got introduced, whereas hook_entity_property_info()
    may be placed in your module's {YOUR_MODULE}.info.inc include file. For details
    have a look at the doxygen documentation and at http://drupal.org/node/878876.
    
  * The module provides API functions allowing modules to create, save, delete
    or to determine access for entities based on any entity type, for which the
    necessary metadata is available. The module comes with metadata for all
    core entity types, contrib modules are supposed to provide metadata on their
    own.

  * The metadata about entity properties contains information about the
    data type and callbacks for how to get and set the data of property. That
    way the data of an entity can be easily reused, e.g. to export into other
    data formats like XML.
 
  * For making use of this metadata the module provides some wrapper classes
    which ease getting and setting values. The wrapper support chained usage for
    retrieving wrappers of entity properties, e.g. to get a node author's mail
    address one could use:
    
       $wrapper = entity_metadata_wrapper('node', $node);
       $wrapper->author->mail->value();
       
       
    To update the user's mail address one could use
    
       $wrapper->author->mail->set('sepp@example.com');
       
       or
       
       $wrapper->author->mail = 'sepp@example.com'; 
       
    The wrappers always return the data as described in the metadata, which may
    be retrieved directly from hook_entity_info() for from the wrapper:
    
       $mail_info = $wrapper->author->mail->info();
       
    However to force getting a sanitized value one can use
       
       $wrapper->title->value(array('sanitize' => TRUE));
       
    to get the sanitized node title. When data is already sanitized, like the
    node body, one possible wants to get the data as output in the browser. For
    that one can enable 'decode', so for sanitized data the tags are stripped
    and the data is decoded before it is returned:
    
       $wrapper->body->value->value(array('decode' => TRUE));
       
    That way one always gets the data as shown to the user. However if you
    really want to get the raw value, even for sanitized textual data, you can
    do so:
    
      $wrapper->body->value->raw();
      
 