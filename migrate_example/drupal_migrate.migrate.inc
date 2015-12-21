<?php
function drupal_migrate_migrate_api(){
  $api = array(
      'api' => 2,
      'groups' => array(
          'Drupal' => array(
          'title' => t('Drupal Migrations'),
        ),
    ),
    'migrations' => array(
      'DrupalUser' => array(
        'class_name' => 'DrupalUsersMigration',
        'group_name' => 'Drupal',
      ),
      'DrupalUserDelete' => array(
        'class_name' => 'DrupalUsersMigrationDelete',
        'group_name' => 'Drupal',
      ),
      'DrupalTaxonomiesTags' => array(
        'class_name' => 'DrupalTaxonomiesMigrationTags',
        'group_name' => 'Drupal',
      ),
      'DrupalNodesArticle' => array(
        'class_name' => 'DrupalNodeMigrationArticle',
        'group_name' => 'Drupal',
      ),
      'DrupalCommerceOrder' => array(
        'class_name' => 'DrupalCommerceOrderMigration',
        'group_name' => 'Drupal',
      ),      
      'DrupalTaxomoniesDelete' => array(
        'class_name' => 'DrupalTaxonomiesMigrationDelete',
        'group_name' => 'Drupal',
      ),
      'DrupalNodeArticlesDelete' => array(
        'class_name' => 'DrupalNodeMigrationArticlesDelete',
        'group_name' => 'Drupal',
      ),      
    ),
  );
  return $api;
}

