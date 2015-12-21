<?php

class DrupalCommerceOrderMigration extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Migrate Drupal Commerce Orders');
 
 
    $query = db_select(OLDBASE.'.commerce_order', 'o');
      $query->fields('o', array('order_id', 'revision_id', 'type', 'uid', 'mail', 'status', 'created', 'changed', 'hostname', 'data'));
      $query->condition('o.status', 'completed', '=');
      $query->condition('o.type', 'commerce_order', '=');
      $query->condition('o.uid', 0, '>');
      //$query->range(0,15);


    $this->source = new MigrateSourceSQL($query);
    $this->destination = new MigrateDestinationEntityAPI('commerce_order', 'commerce_order');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'order_id' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'ID of destination commerce order',
          'alias' => 'o',
        )
      ),
      MigrateDestinationEntityAPI::getKeySchema('commerce_order', 'commerce_order')
    );
 
    // Make the mappings
    $this->addFieldMapping('order_number', 'order_id');
    $this->addFieldMapping('revision_id', 'revision_id');
    $this->addFieldMapping('type', 'type');
    $this->addFieldMapping('mail', 'mail');
    $this->addFieldMapping('status', 'status');    
    $this->addFieldMapping('created', 'created');
    $this->addFieldMapping('changed', 'changed');
    $this->addFieldMapping('hostname', 'hostname');
    $this->addFieldMapping('data', 'data');


  }

    public function complete($entity, stdClass $row) {
      //new order
      $order = commerce_order_load($entity->order_id);

      // For relationship order-user -------------------------------------
      $userObj = user_load_by_mail($row->mail);

      if($userObj !== false){
        //$newUid = array(LANGUAGE_NONE => array(array('uid' => current($userObj)->uid)));
        $order->uid = $userObj->uid;
      } 
      // -----------------------------------------------------------------   

      $queryPP = db_select(OLDBASE.'.field_data_field_preferred_prefix', 'pp');
      $queryPP->addField('pp', 'field_preferred_prefix_value');
      $queryPP->condition('pp.entity_id', $row->order_id);  
      $queryPP->condition('pp.entity_type', 'commerce_order');  
      $rowPP=$queryPP->execute()->fetchAll();

      
      if(current($rowPP)!== false){
        $order->field_preferred_prefix = array(LANGUAGE_NONE => array(array('value' => current($rowPP)->field_preferred_prefix_value))); 
      }

      
      commerce_order_save($order); //register user in drupal

    }

}

