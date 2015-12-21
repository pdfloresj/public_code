<?php

class DrupalUsersMigration extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Migrate Drupal users');
 
    $source_fields = array(
      'uid' => t('User ID'),
      'roles' => t('The set of roles assigned to a user.'),
    );
 
    $query = db_select(OLDBASE.'.users', 'u');
      $query->fields('u', array('uid', 'name', 'pass', 'mail', 'created', 'access', 'login', 'status', 'init'));
      $query->condition('u.name', array('admin', 'rgates'), 'NOT IN');
      $query->condition('u.uid', 0, '>');
      //$query->range(1,10);

      //var_dump(user_load('1')); die();
     

    $this->source = new MigrateSourceSQL($query, $source_fields);
    $this->destination = new MigrateDestinationUser();

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'uid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'D6 Unique User ID',
          'alias' => 'u',
        )
      ),
      MigrateDestinationUser::getKeySchema()
    );
 
    // Make the mappings
    $this->addFieldMapping('name', 'name')->dedupe('users', 'name');
    $this->addFieldMapping('pass', 'pass');
    $this->addFieldMapping('mail', 'mail')->dedupe('users', 'mail');
    $this->addFieldMapping('language')->defaultValue('');
    $this->addFieldMapping('theme')->defaultValue('');
    $this->addFieldMapping('signature')->defaultValue('');
    $this->addFieldMapping('signature_format')->defaultValue('filtered_html');
    $this->addFieldMapping('created', 'created');
    $this->addFieldMapping('access', 'access');
    $this->addFieldMapping('login', 'login');
    $this->addFieldMapping('status', 'status');
    $this->addFieldMapping('picture')->defaultValue(0);
    $this->addFieldMapping('init', 'init');
    $this->addFieldMapping('timezone')->defaultValue(NULL);
    $this->addFieldMapping('path')->issueGroup(t('DNM'));
    $this->addFieldMapping('pathauto_perform_alias')->defaultValue('1');
    $this->addFieldMapping('roles', 'roles');
    
  }

    public function complete($entity, stdClass $row) {
      // Reset password hash back to the source hash; when Migrate saves the user
      // entity after the prepare() method is complete, Drupal hashes the hash,
      // meaning we need to set it back to the original hash.
      // SEE: https://drupal.org/node/1349758
      db_update('users')
        ->fields(array('pass' => $row->pass))
        ->condition('uid', $entity->uid)
        ->execute();

      //Other fields
      $existing = user_load($entity->uid);

      $queryPBA = db_select(OLDBASE.'.field_data_field_primary_business_activity', 'pba');
      $queryPBA->addField('ttd', 'name', 'pbactivity');
      $queryPBA->leftJoin(OLDBASE.'.taxonomy_term_data', 'ttd', 'ttd.tid=pba.field_primary_business_activity_tid');
      $queryPBA->condition('pba.entity_type', 'user', '=');  
      $queryPBA->condition('pba.entity_id', $row->uid, '=');  
      $rowPBA=$queryPBA->execute()->fetchAll();

           
      if(current($rowPBA)!== false){
        $PBATermId = taxonomy_get_term_by_name(current($rowPBA)->pbactivity,'Drupal_primary_business_activity'); 

        if($PBATermId!==false){
          $newPBA = array(LANGUAGE_NONE => array(array('tid' => current($PBATermId)->tid)));
          $newuser['field_primary_business_activity']=$newPBA;
        } 
      }

      
      $user = user_save($existing, $newuser); //register user in drupal

    }
 
  public function prepareRow($current_row) {
    $source_id = $current_row->uid;
    $query = db_select(OLDBASE.'.users_roles', 'r')
      ->fields('r', array('uid', 'rid'))
      ->condition('r.uid', $source_id, '=');
    $results = $query->execute();
    $roles = array('2' => '2');
    foreach ($results as $row) {
      $roles[$row->rid] = $row->rid;
    }
    $current_row->roles = $roles;
    return TRUE;
    // return FALSE if you wish to skip a particular row
  }

}


class DrupalUsersMigrationDelete extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Migrate Drupal users');
 
    $source_fields = array(
      'uid' => t('User ID'),
      'roles' => t('The set of roles assigned to a user.'),
    );
 

    $query = db_select(OLDBASE.'.users', 'u');
      $query->fields('u', array('uid', 'name', 'pass', 'mail', 'created', 'access', 'login', 'status', 'init'));
      $query->addField('un', 'uid', 'uidn');
      $query->leftJoin('.users', 'un', 'un.name=u.name');
      $query->condition('u.name', array('admin', 'rgates'), 'NOT IN');
      $query->condition('u.uid', 0, '>');
      $query->condition('u.name', 'frontend', '!=');
      $query->range(1,1);

    $this->source = new MigrateSourceSQL($query, $source_fields);
    $this->destination = new MigrateDestinationUser();

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'uid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'D6 Unique User ID',
          'alias' => 'u',
        )
      ),
      MigrateDestinationUser::getKeySchema()
    );
    //$this->addFieldMapping('uid', 'uidn');
  }

  public function prepareRow($current_row) {
      // Delete Users ---------------------------------------------
      $query = db_select('.users', 'u');
      $query->fields('u', array('uid', 'name'));
      $query->condition('u.uid', 0, '!=');
      $query->condition('u.uid', 1, '!=');
      $query->condition('u.uid', 9, '!=');
      $query->condition('u.uid', 10, '!=');
      $userArray = $query->execute()->fetchAll();

      foreach ($userArray as $userRow) {

        user_delete($userRow->uid);

      }
      // -----------------------------------------------------------------------------------------------------------------
  }

}
