<?php

/**
 * @file
 */
function hide_author_admin_settings_page() {
  $form = array();
  $options = array('node' => t('Nodes'), 'comment' => t('Comments'));
  $form['hide_author_enable'] = array(
    '#title' => t('Enable'),
    '#description' => t('Turn on the hide author functionality for the site'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => variable_get('hide_author_enable', $options),
  );
  $form['hide_author_admin_save'] = array(
    '#title' => t('Save Author'),
    '#description' => t('Allow the module to save the anonymous author for administrative review'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => variable_get('hide_author_admin_save', $options),
  );

  return system_settings_form($form);
}

function hide_author_admin_nodes_page() {
  $header = array(
    'eid' => array('data' => t('NID'), 'field' => 'eid', 'sort' => 'desc'),
    'Title' => array('data' => t('Title'), 'field' => 'eid'),
    'uid' => array('data' => t('User'), 'field' => 'uid'),
  );
  $query = db_select('hide_author', 'ha');
  $query->join('node', 'n', 'ha.eid = n.nid');
  $query->join('users', 'u', 'ha.uid = u.uid');
  $query->fields('ha', array('eid', 'uid'))
    ->condition('ha.entity', 'node','=')
    ->fields('n', array('title'))
    ->fields('u', array('name'))
    ->extend('PagerDefault')
    ->limit(50)
    ->extend('TableSort')
    ->orderByHeader($header);
  $result = $query->execute();

  $rows = array();
  foreach ($result as $record) {
    $rows[$record->eid]['eid'] = $record->eid;
    $rows[$record->eid]['title'] = l($record->title, 'node/' . $record->eid);
    $rows[$record->eid]['uid'] = l($record->name, 'user/' . $record->uid);
  }
 
  $form['rules'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('There are currently no nodes with a hidden author.'),
  );

  return $form;
}

function hide_author_admin_comments_page() {
  $header = array(
    'eid' => array('data' => t('CID'), 'field' => 'eid', 'sort' => 'desc'),
    'Subject' => array('data' => t('Subject'), 'field' => 'eid'),
    'uid' => array('data' => t('User'), 'field' => 'uid'),
  );

  $query = db_select('hide_author', 'ha');
  $query->join('comment', 'c', 'ha.eid = c.cid');
  $query->join('users', 'u', 'ha.uid = u.uid');
  $query->fields('ha', array('eid', 'uid'))
    ->condition('ha.entity', 'comment','=')
    ->fields('c', array('subject'))
    ->fields('u', array('name'))
    ->extend('PagerDefault')
    ->limit(50)
    ->extend('TableSort')
    ->orderByHeader($header);
  $result = $query->execute();

  $rows = array();
  foreach ($result as $record) {
    $rows[$record->eid]['eid'] = $record->eid;
    $rows[$record->eid]['subject'] = l($record->subject, 'comment/' . $record->eid);
    $rows[$record->eid]['uid'] = l($record->name, 'user/' . $record->uid);
  }
 
  $form['rules'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('There are currently no comments with a hidden author.'),
  );

  return $form;
}
