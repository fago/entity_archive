$Id$

Entity APIs
------------
by Wolfgang Ziegler, nuppla@zites.net


This are API modules, so only install them if you are instructed to do so, you
want to run the tests or to force using modules to use the latest version of the
entity CRUD API.


This README is for interested developers. If you are not interested in developing,
you may stop reading now.


--------------------------------------------------------------------------------
                              Entity CRUD API
--------------------------------------------------------------------------------



Relationship to the Extendable Object Faces (API)
-------------------------------------------------
http://drupal.org/project/faces

 * Entities are extendable with the help of faces by default. So to make this
   possible you also have to ship with the "faces.inc" include.
   
 * If you don't want to, you may edit your copy of entity.inc and make the
   class "EntityDB" not extending "FacesExtendable". Also remove 'faces' from your
   modules autoloader (see below). That's it. 
   That way you can ship with "entity.inc" without having to ship with "faces.inc".


How to use the entity CRUD API without introducing a dependency?
----------------------------------------------------------------

 * Add the most recent "entity.inc" include file to your module.
 
 * Add the most recent "faces.inc" include file from
   http://drupal.org/project/faces, if you have not done yet.

 * Add the following code at the top of your module. Replace MODULE with your
   module's name.

CODE:
--------------------------------------------------------------------------------
 
 spl_autoload_register('MODULE_autoload');

/**
 * Autoload API includes. Note that the code registry autoload is used only
 * by the providing API module.
 */
function MODULE_autoload($class) {
  if (stripos($class, 'faces') === 0) {
    module_load_include('inc', 'MODULE', 'faces');
  }
  if (stripos($class, 'entity') === 0) {
    module_load_include('inc', 'MODULE', 'entity');
  }
}

--------------------------------------------------------------------------------

 * That way the include is autoloaded once a class of it is used. Once this
   module is installed and enabled, the version of this module is used, because
   it uses the code registry and its autoloader comes first.
   
 
 
 
 How to add a new entity?
 ------------------------
 
  * You might want to study the code of the "entity_test.module".
  
  * Describe your entities db table as usual in hook_schema().
  
  * You may use the "EntityDB" class directly or extend it with your own class.
    To see how to provide a separate class have a look at the "EntityClass" from
    the "entity_test.module". 
  
  * Implement hook_entity_info() for your entity. At least specifiy the entity
    class, the controller class of this API, your db table and your object's
    primary key field.
    Again just look at "entity_test.module"'s hook_entity_info() for guidance.
    If you don't want to create a separate entity class, just set 'entity class'
    to 'EntityDB'.
    
  * If you want your entity to be fieldable just set 'fieldable' in hook_entity_info
    to TRUE. The field API attachers are called automatically in the entities
    CRUD functions then.
    
  * Schema fields marked as 'serialzed' are automatically unserialized upon
    loading. If the 'merge' attribute is also set to TRUE the unserialized data
    is automatically "merged" into the entity.

    
    

--------------------------------------------------------------------------------
                              Entity Metadata
--------------------------------------------------------------------------------

  * This module introduces a unique place for metadata about entity properties.
    For that hook_entity_info() already used by core is extended, for details
    have a look at the doxygen documentation. (not yet there)

  * The metadata about entity properties contains information about the
    data type and callbacks for how to get and set the data of property. That
    way the data of an entity can be easily reused, e.g. to export into other
    data formats like XML.

  * The module provides this metadata for all core modules, contrib modules
    should provide the data on their own. For that hook_entity_info_alter() can
    be implemented. When only used for the metadata this implementation may
    reside in a {YOUR_MODULE}.info.inc include file, which is automatically
    included once this module is active and the hook is invoked.

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
    
       $wrapper->body->value(array('decode' => TRUE));
       
    That way one always gets the data as shown to the user. However if you
    really want to get the raw value, even for sanitized textual data, you can
    do so:
    
      $wrapper->body->raw();
      
