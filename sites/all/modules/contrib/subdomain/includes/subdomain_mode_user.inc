<?php

/**
 * @file
 * Subdomain engine user mode subclass
 */


/**
 * Description of subdomain_user
 *
 * @author setvik
 */
class subdomain_mode_user extends subdomain_engine {

  function type() {
    return 'user';
  }

  function base_path($sid) {
    return 'user/' . $sid;
  }

  function static_form_text() {
    return t('My homepage');
  }

  function is_entry_form($form_id) {
    return in_array($form_id, array('user_register_form', 'user_profile_form'));
  }

  function get_content_subdomain($nid) {
    $subdomains = &drupal_static(__CLASS__ . '__' . __METHOD__ . '__filtered', NULL);

    if (!isset($subdomains[$nid])) {
      $subdomains[$nid] = db_query("SELECT subdomain
          FROM {subdomain} s
          INNER JOIN {node} n ON s.sid = n.uid
          WHERE nid = :nid AND type NOT IN (:types)",
        array(':nid' => $nid, ':types' => _subdomain_filtered_content_types()))
      ->fetchField();
    }
    
    return $subdomains[$nid];
  }

  function entry_form_alter(&$form, $form_id) {
    $default = subdomain_get_subdomain($form['#user']->uid);
    $form['subdomain_exists'] = array(
      '#type' => 'value',
      '#value' => !empty($default),
    );

    $form['#validate'][] = '_subdomain_validate_subdomain';

    if ($this->source == 'custom' && !($form_id == 'user_register' && !user_access('create subdomains'))) {
      if (isset($form['account'])) {
        $parent = &$form['account'];
      }
      else {
        $parent = &$form;
      }
      $parent['name']['#weight'] = -5.1;

      $editable = $form_id == 'user_register' && user_access('create subdomains') || user_access('edit subdomains');

      $parent['subdomain'] = _subdomain_get_custom_form_field($default, $editable);
    }
    _subdomain_add_js_validation($form['#user']->uid);
  }
  
  function save($user) {
    $this->save_record($user['uid'], $user[$this->form_field]);
  }
  
  function delete($user) {
    $this->delete_record($user->uid);
  }

  function url_alter_helper(&$subdomain, &$path, $original_path, $outbound = TRUE) {
    if ($uid = _subdomain_id_from_path('uid', $original_path)) {
      $subdomain = subdomain_get_subdomain($uid);
      if (!empty($subdomain) && $path == $this->base_path($uid) && $outbound) {
        $path = '';
      }
    }
    elseif ($nid = _subdomain_id_from_path('nid', $original_path)) {
      $subdomain = subdomain()->get_content_subdomain($nid);
    }
  }

  /**
   * Bulk generate subdomains from term name
   */
  function bulk_generate() {
    $result = db_query("SELECT uid, name FROM {users} u LEFT JOIN {subdomain} s on u.uid = s.sid WHERE s.sid is NULL AND u.uid > 0");
    $count = 0;
    foreach ($result as $row) {
      $account = user_load($row->uid);
      if (user_access('create subdomains', $account)) {
        $this->save_record($row->uid, $row->name, TRUE);
        $count += 1;
      }
    }
    drupal_set_message(format_plural($count, '1 new subdomain created.', '@count new subdomains created.'));
  }
}