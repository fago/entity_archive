$Id$

Entity APIs
------------
by Wolfgang Ziegler, nuppla@zites.net

This is an API module, only install if you want to run the tests or to force using
modules to use the latest version. Modules using the API should just ship with
the include file.

This README is for interested developers. If you are not interested in developing,
you may stop reading now.



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
