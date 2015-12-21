<?php

//Tags
class DrupalTaxonomiesMigrationTags extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Migrate taxonomy terms');
    //$idlist = $this->getOption('idlist');

    $query = db_select(OLDBASE.'.taxonomy_term_data', 'td');
    $query->fields('td', array('tid', 'name', 'description', 'weight', 'format'));
    $query->addField('th', 'parent');
    $query->addField('tdt', 'field_taxonomy_display_title_value');
    $query->addField('tdt', 'field_taxonomy_display_title_format');
    // Order by parent so no term can be created when the parent doesn't exist yet.
    $query->orderBy('th.parent');
    // Join to the hierarchy so we can sort on parent, but we'll pull the
    // actual parent values in separately in case there are multiples.
    $query->leftJoin(OLDBASE.'.taxonomy_term_hierarchy', 'th', 'td.tid=th.tid');   
    $query->leftJoin(OLDBASE.'.field_revision_field_taxonomy_display_title', 'tdt', 'td.tid=tdt.entity_id');
     $query->innerJoin(OLDBASE.'.taxonomy_vocabulary', 'v', 'td.vid=v.vid');
    $query->condition('v.machine_name', 'tags', '=');

    $this->source = new MigrateSourceSQL($query);

    //$term_options['allow_duplicate_terms'] = TRUE;   
    $this->destination = new MigrateDestinationTerm('tags');

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'tid' => array(
            'type' => 'int',
            'unsigned' => TRUE,
            'not null' => TRUE,
            'description' => 'Drupal 6 term ID',
            'alias' => 'td',
          )
        ),
        MigrateDestinationTerm::getKeySchema()
    );

    $this->addFieldMapping('name', 'name');
    $this->addFieldMapping('description', 'description');
    $this->addFieldMapping('format', 'format');
    $this->addFieldMapping('weight', 'weight');
    $this->addFieldMapping('parent', 'parent');

    $this->addFieldMapping('field_taxonomy_display_title', 'field_taxonomy_display_title_value');

    $this->addFieldMapping('field_taxonomy_display_title:format')
        ->defaultValue('full_html');

    $this->addFieldMapping('path')->issueGroup(t('DNM'));
    $this->addFieldMapping('pathauto_perform_alias')->defaultValue('1');
    

  }

 public function prepareRow($row) {
  if (parent::prepareRow($row) === FALSE) {
    return FALSE;
  }

  // Add the (potentially multiple) parents.
  $result =  db_select(OLDBASE.'.taxonomy_term_hierarchy', 'th')
      ->fields('th', array('parent'))
      ->condition('tid', $row->tid)
      ->execute();
  $row->parent = array();
  foreach ($result as $parent_row) {
    $row->parent[] = $parent_row->parent;
  }

  // Creation of taxonomy vocabulary ---------------------------------------------
  $queryVoc = db_select(OLDBASE.'.taxonomy_vocabulary', 'voc');
  $queryVoc->fields('voc', array('name', 'machine_name', 'description', 'hierarchy', 'module', 'weight'));
  $vocArray = $queryVoc->execute()->fetchAll();

  foreach ($vocArray as $voc) {
    $existVoc = taxonomy_vocabulary_machine_name_load($voc->machine_name);
    // Creation of vocabulary
    if ($existVoc === false){
      taxonomy_vocabulary_save($voc);
    }
  }
  // -----------------------------------------------------------------------------------------------------------------

 // $this->version->getSourceValues($row, $row->tid);
  return TRUE;
}

  public function complete($entity, stdClass $row) {
    //Fill all fields for Drupal author
    $queryPTHA = db_select(OLDBASE.'.url_alias', 'pth');
    $queryPTHA->addField('pth', 'alias');
    $queryPTHA->condition('pth.source', 'taxonomy/term/'.$row->tid, '=');
    $rowPTHA=$queryPTHA->execute()->fetchAll();

    if(current($rowPTHA)!== false){

        $tid = $entity->tid;
        $urlAlias = drupal_lookup_path('alias',"taxonomy/term/".$tid);
        // query to get the pid

        $queryPTHAN = db_select('.url_alias', 'pth');
        $queryPTHAN->addField('pth', 'pid');
        $queryPTHAN->condition('pth.alias', $urlAlias, '=');
        $rowPTHAN=$queryPTHAN->execute()->fetchAll();
        $pid = current($rowPTHAN)->pid;
        path_delete($pid); 
      //-------------------------------------------------------------------------------------     
      foreach($rowPTHA as $value){
        $pathC = array(
            //'pid'         =>  $pid,
            'source'        =>  "taxonomy/term/".$tid,
            'alias'         =>  $value->alias,
        );
        path_save($pathC);
      }
    }
  }
  
}


//Delete Taxonomy
class DrupalTaxonomiesMigrationDelete extends Migration {
    public function __construct($arguments) {
      parent::__construct($arguments);
      $this->description = t('Migrate taxonomy terms');

      $query = db_select(OLDBASE.'.taxonomy_term_data', 'td');
      $query->fields('td', array('tid', 'name', 'description', 'weight', 'format'));
      $query->range(1,1);

      $this->source = new MigrateSourceSQL($query);

      $this->destination = new MigrateDestinationTerm('Drupal_aum_invested_in_Drupals_etns');

      $this->map = new MigrateSQLMap($this->machineName,
          array(
            'tid' => array(
              'type' => 'int',
              'unsigned' => TRUE,
              'not null' => TRUE,
              'description' => 'Drupal 6 term ID',
              'alias' => 'td',
            )
          ),
          MigrateDestinationTerm::getKeySchema()
      );

    }

   public function prepareRow($current_row) {
      // Delete of taxonomy vocabulary ---------------------------------------------
      $queryVoc = db_select('.taxonomy_term_data', 'voc');
      $queryVoc->fields('voc', array('tid','name'));
      $vocArray = $queryVoc->execute()->fetchAll();

      foreach ($vocArray as $voc) {
          taxonomy_term_delete($voc->tid);
      }
      // -----------------------------------------------------------------------------------------------------------------
  }
  
}
