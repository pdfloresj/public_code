<?php
//Articles
class DrupalNodeMigrationArticle extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Migrate Drupal Articles content');

    $query = db_select(OLDBASE.'.node', 'nd');
    $query->fields('nd', array('nid', 'vid', 'type', 'language', 'title', 'status', 'created', 'changed', 'comment', 'promote', 'sticky', 'tnid', 'translate'));
    $query->orderBy('nd.nid');
    $query->condition('nd.type', 'Drupal_general_article', '=');
    $query->condition('nd.nid', 0, '>');
    //$query->range(1,10);
    
    $this->source = new MigrateSourceSQL($query);

    $node_options = MigrateDestinationNode::options(NULL, 'filtered_html');
    $term_options['allow_duplicate_terms'] = TRUE;   
//    $node_options = MigrateDestinationNode::options(LANGUAGE_NONE, 'filtered_html');

    $this->destination = new MigrateDestinationNode('Drupal_articles');

    $this->map = new MigrateSQLMap($this->machineName,
          array(
          'nid' => array(
            'type' => 'int',
            'unsigned' => TRUE,
            'not null' => TRUE,
            'description' => 'ID of destination node revision',
            'alias' => 'nd',
          ),
        ),
        MigrateDestinationNode::getKeySchema()
    );

    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('language', 'language');
    //$this->addFieldMapping('uid', 1);
    $this->addFieldMapping('status', 'status');
    $this->addFieldMapping('created', 'created');    
    $this->addFieldMapping('changed', 'changed'); 
    $this->addFieldMapping('comment', 'comment'); 
    $this->addFieldMapping('promote', 'promote'); 
    $this->addFieldMapping('sticky', 'sticky'); 
    $this->addFieldMapping('tnid', 'tnid'); 
    $this->addFieldMapping('translate', 'translate');  

  }

  public function complete($entity, stdClass $row) {
  //Fill all fields for Drupal author
    $node = node_load($entity->nid);

    $queryBD = db_select(OLDBASE.'.field_data_body', 'db');
    $queryBD->fields('db', array('body_value','body_summary','body_format'));
    $queryBD->condition('db.bundle', 'Drupal_general_article', '=');
    $queryBD->condition('db.entity_id', $row->nid, '=');
    $rowBD=$queryBD->execute()->fetchAll();

    $queryEA = db_select(OLDBASE.'.field_data_field_Drupal_author', 'ea');
    $queryEA->addField('ea', 'field_Drupal_author_target_id', 'nid');
    $queryEA->condition('ea.entity_id', $row->nid, '=');
    $rowEA=$queryEA->execute()->fetchAll();

    $queryDT = db_select(OLDBASE.'.field_data_field_date', 'dt');
    $queryDT->addField('dt', 'field_date_value');
    $queryDT->condition('dt.entity_id', $row->nid, '=');
    $rowDT=$queryDT->execute()->fetchAll();

    $queryYA = db_select(OLDBASE.'.field_data_field_yahoo_action', 'ya');
    $queryYA->addField('ya', 'field_yahoo_action_value');
    $queryYA->condition('ya.entity_id', $row->nid, '=');
    $rowYA=$queryYA->execute()->fetchAll();

    $queryRE = db_select(OLDBASE.'.field_data_field_related_Drupals', 're');
    $queryRE->addField('re', 'field_related_Drupals_value');
    $queryRE->condition('re.entity_type', 'node', '='); 
    $queryRE->condition('re.entity_id', $row->nid, '=');
    $queryRE->orderBy('re.revision_id','DESC');
    $rowRE=$queryRE->execute()->fetchAll();

    $queryFC = db_select(OLDBASE.'.field_data_field_Drupal_categories', 'fc');
    $queryFC->addField('ttd', 'name', 'fcategory');
    $queryFC->leftJoin(OLDBASE.'.taxonomy_term_data', 'ttd', 'ttd.tid=fc.field_Drupal_categories_tid');
    $queryFC->condition('fc.entity_id', $row->nid, '=');  
    $rowFC=$queryFC->execute()->fetchAll();

    $queryFT = db_select(OLDBASE.'.field_data_field_tags', 'ft');
    $queryFT->addField('ttd', 'name', 'ftags');
    $queryFT->leftJoin(OLDBASE.'.taxonomy_term_data', 'ttd', 'ttd.tid=ft.field_tags_tid');
    $queryFT->condition('ft.entity_id', $row->nid, '=');  
    $rowFT=$queryFT->execute()->fetchAll();

    if(current($rowFT)!== false){
      $i=0;
      foreach($rowFT as $value){   
        $PBATermId = taxonomy_get_term_by_name($value->ftags,'tags'); 

        if($PBATermId!==false){
          if(isset(current($PBATermId)->tid)){
            $node->field_tags[$node->language][$i]['tid']=current($PBATermId)->tid;
          }
        }

        $i++;
      }
    }

    if(current($rowFC)!== false){
      $PBATermId = taxonomy_get_term_by_name(current($rowFC)->fcategory,'Drupal_sections'); 

      if($PBATermId!==false){
        if(isset(current($PBATermId)->tid)){
          $node->field_Drupal_categories[$node->language][0]['tid']=current($PBATermId)->tid;
        }
      } 
    }

    if(current($rowRE)!== false){

      $arrRE=explode( ', ', current($rowRE)->field_related_Drupals_value);
      $i=0;
      foreach($arrRE as $value){
        $queryNRE = db_select('.node', 'nd');
        $queryNRE->addField('nd', 'nid');
        $queryNRE->condition('nd.title', $value, '=');
        $rowNRE=$queryNRE->execute()->fetchAll();
        if(current($rowNRE)!== false){
          if(isset(current($rowNRE)->nid)){
            $node->field_related_Drupals[$node->language][$i]['target_id']= current($rowNRE)->nid;
            $i++;
          }
        }
      }
    }

    if(current($rowYA)!== false){
        $node->field_yahoo_action[$node->language][0]['value']= current($rowYA)->field_yahoo_action_value;
    }

    if(current($rowDT)!== false){
        $node->field_date[$node->language][0]['value']= current($rowDT)->field_date_value;
    }

    if(current($rowEA)!== false){
      $i=0;
      foreach($rowEA as $value){
        $queryEAN = db_select(OLDBASE.'.node', 'ndo');
        $queryEAN->addField('ndn', 'nid');
        $queryEAN->leftJoin('.node', 'ndn', 'ndn.title=ndo.title');
        $queryEAN->condition('ndo.type', 'Drupal_author', '=');    
        $queryEAN->condition('ndn.type', 'Drupal_author', '=');
        $queryEAN->condition('ndo.nid', $value->nid, '=');
        $rowEAN=$queryEAN->execute()->fetchAll();

        if(empty($rowEAN)!== true){
            $node->field_Drupal_author[$node->language][$i]['target_id']= current($rowEAN)->nid;
        }
        else{
          $queryEAT = db_select('.node', 'ndn');
          $queryEAT->addField('ndn', 'nid');
          $queryEAT->condition('ndn.title', 'Drupal.com Staff', '=');    
          $queryEAT->condition('ndn.type', 'Drupal_author', '=');
          $rowEAT=$queryEAT->execute()->fetchAll();

          $node->field_Drupal_author[$node->language][$i]['target_id']= current($rowEAT)->nid;
        }
        $i++;
      }
    }

    if(current($rowBD)!== false){
        $node->body[$node->language][0]['value']= current($rowBD)->body_value;
        $node->body[$node->language][0]['summary']= current($rowBD)->body_summary;
        $node->body[$node->language][0]['format']= current($rowBD)->body_format;
    } 

    $queryFBP = db_select(OLDBASE.'.field_data_field_image', 'fp');
    $queryFBP->fields('fp', array('field_image_fid', 'field_image_width','field_image_height'));
    $queryFBP->fields('fm', array('filename','uri','filemime','filesize','timestamp','type'));
    $queryFBP->leftJoin(OLDBASE.'.file_managed', 'fm', 'fm.fid=fp.field_image_fid');
    $queryFBP->condition('fp.entity_id', $row->nid, '=');
    $rowFBP=$queryFBP->execute()->fetchAll();

    if(current($rowFBP)!== false){
      // Start: Associate the file to image -------------------------------------------

      $result = db_query('SELECT f.fid FROM file_managed f WHERE f.filename = :filename ', array(':filename' => current($rowFBP)->filename));
      $fidb= $result->fetchColumn(0);
      
      if (!$fidb){
        if (file_exists(current($rowFBP)->uri)){
          $fileb = new stdClass;
          $fileb->uid = 1;
          $fileb->filename = current($rowFBP)->filename;
          $fileb->uri = current($rowFBP)->uri;
          $fileb->filemime = current($rowFBP)->filemime;
          $fileb->filesize = filesize(current($rowFBP)->uri);
          $fileb->status = FILE_STATUS_PERMANENT;
          $fileb=file_save($fileb);
          $node->field_hero_image[$node->language][0] = (array)$fileb;
        }
      } else {
        $filebon = file_load($fidb);
        if($filebon->uri != current($rowFBP)->uri){
          $resulto = db_query('SELECT f.fid FROM file_managed f WHERE f.uri = :uri ', array(':uri' => current($rowFBP)->uri));
          $fidbo= $resulto->fetchColumn(0);
          if(!$fidbo){
            if (file_exists(current($rowFBP)->uri)){
              $fileb = new stdClass;
              $fileb->uid = 1;
              $fileb->filename = current($rowFBP)->filename;
              $fileb->uri = current($rowFBP)->uri;
              $fileb->filemime = current($rowFBP)->filemime;
              $fileb->filesize = filesize(current($rowFBP)->uri);
              $fileb->status = FILE_STATUS_PERMANENT;
              $fileb=file_save($fileb);
              $node->field_hero_image[$node->language][0] = (array)$fileb;
            }
          }
          else{
            $filebo = file_load($fidbo);
            $node->field_hero_image[$node->language][0] = (array)$filebo;
          }
        }
        else{
          $node->field_hero_image[$node->language][0] = (array)$filebon;
        }
      }
    }

    $queryFPC = db_select(OLDBASE.'.field_data_field_podcasts', 'fp');
    $queryFPC->fields('fp', array('field_podcasts_fid', 'field_podcasts_display'));
    $queryFPC->fields('fm', array('filename','uri','filemime','filesize','timestamp','type'));
    $queryFPC->leftJoin(OLDBASE.'.file_managed', 'fm', 'fm.fid=fp.field_podcasts_fid');
    $queryFPC->condition('fp.entity_id', $row->nid, '=');
    $rowFPC=$queryFPC->execute()->fetchAll();

    if(current($rowFPC)!== false){
      // Start: Associate the file to image -------------------------------------------

      $result = db_query('SELECT f.fid FROM file_managed f WHERE f.filename = :filename ', array(':filename' => current($rowFPC)->filename));
      $fidb= $result->fetchColumn(0);
      
      if (!$fidb){
        if (file_exists(current($rowFPC)->uri)){
          $fileb = new stdClass;
          $fileb->uid = 1;
          $fileb->filename = current($rowFPC)->filename;
          $fileb->uri = current($rowFPC)->uri;
          $fileb->filemime = current($rowFPC)->filemime;
          $fileb->filesize = filesize(current($rowFPC)->uri);
          $fileb->status = FILE_STATUS_PERMANENT;
          $fileb=file_save($fileb);
          $node->field_podcasts[$node->language][0] = (array)$fileb;
        $node->field_podcasts[$node->language][0]['display'] = current($rowFPC)->field_podcasts_display;
        }
      } else {
        $fileb = file_load($fidb);
        $node->field_podcasts[$node->language][0] = (array)$fileb;
        $node->field_podcasts[$node->language][0]['display'] = current($rowFPC)->field_podcasts_display;
      }
    }

    node_save($node);

    $queryPTHA = db_select(OLDBASE.'.url_alias', 'pth');
    $queryPTHA->addField('pth', 'alias');
    $queryPTHA->condition('pth.source', 'node/'.$row->nid, '=');
    $rowPTHA=$queryPTHA->execute()->fetchAll();

    if(current($rowPTHA)!== false){

        $nid = $entity->nid;
        $urlAlias = drupal_lookup_path('alias',"node/".$nid);
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
            'source'        =>  "node/".$nid,
            'alias'         =>  $value->alias,
        );
        path_save($pathC);
      }
    }
  }

}

class DrupalNodeMigrationAuthorDelete extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Migrate Drupal Author');
 
    $query = db_select(OLDBASE.'.node', 'nd');
    $query->fields('nd', array('nid', 'vid'));
    $query->orderBy('nd.nid');
    $query->condition('nd.type', 'Drupal_author', '=');
    $query->range(1,1);

    $this->source = new MigrateSourceSQL($query);

    $node_options = MigrateDestinationNode::options(NULL, 'filtered_html');
    $term_options['allow_duplicate_terms'] = TRUE;   
//    $node_options = MigrateDestinationNode::options(LANGUAGE_NONE, 'filtered_html');

    $this->destination = new MigrateDestinationNode('Drupal_author');

    $this->map = new MigrateSQLMap($this->machineName,
          array(
          'nid' => array(
            'type' => 'int',
            'unsigned' => TRUE,
            'not null' => TRUE,
            'description' => 'ID of destination node revision',
            'alias' => 'nd',
          ),
        ),
        MigrateDestinationNode::getKeySchema()
    );
    //$this->addFieldMapping('uid', 'uidn');
  }

  public function prepareRow($current_row) {
      // Delete Authors ---------------------------------------------
      $query = db_select('.node', 'nd');
      $query->fields('nd', array('nid', 'title'));
      //$query->condition('nd.type', array('Drupal_author','Drupal_articles'), 'IN');
      $query->condition('nd.type', 'Drupal_author', '=');
      $query->condition('nd.nid', 1, '!=');

      $nodeArray = $query->execute()->fetchAll();


      foreach ($nodeArray as $nodeRow) {

        node_delete($nodeRow->nid);

      }
      // -----------------------------------------------------------------------------------------------------------------
  }

}

class DrupalNodeMigrationArticlesDelete extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Migrate Drupal Author');
 
    $query = db_select(OLDBASE.'.node', 'nd');
    $query->fields('nd', array('nid', 'vid'));
    $query->orderBy('nd.nid');
    $query->condition('nd.type', 'Drupal_general_article', '=');
    $query->range(1,1);

    $this->source = new MigrateSourceSQL($query);

    $node_options = MigrateDestinationNode::options(NULL, 'filtered_html');
    $term_options['allow_duplicate_terms'] = TRUE;   
//    $node_options = MigrateDestinationNode::options(LANGUAGE_NONE, 'filtered_html');

    $this->destination = new MigrateDestinationNode('Drupal_author');

    $this->map = new MigrateSQLMap($this->machineName,
          array(
          'nid' => array(
            'type' => 'int',
            'unsigned' => TRUE,
            'not null' => TRUE,
            'description' => 'ID of destination node revision',
            'alias' => 'nd',
          ),
        ),
        MigrateDestinationNode::getKeySchema()
    );
    //$this->addFieldMapping('uid', 'uidn');
  }

  public function prepareRow($current_row) {
      // Delete Authors ---------------------------------------------
      $query = db_select('.node', 'nd');
      $query->fields('nd', array('nid', 'title'));
      //$query->condition('nd.type', array('Drupal_author','Drupal_articles'), 'IN');
      $query->condition('nd.type', 'Drupal_articles', '=');
      $query->condition('nd.nid', 1, '!=');

      $nodeArray = $query->execute()->fetchAll();


      foreach ($nodeArray as $nodeRow) {

        node_delete($nodeRow->nid);

      }
      // -----------------------------------------------------------------------------------------------------------------
  }

}

