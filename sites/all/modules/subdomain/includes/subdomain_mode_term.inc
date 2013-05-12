<?php

/**
 * @file
 * Subdomain engine term mode subclass
 */


/**
 * Description of subdomain_term
 *
 * @author setvik
 */
class subdomain_mode_term extends subdomain_engine {

  function type() {
    return 'term';
  }

  function base_path($sid) {
    return 'taxonomy/term/' . $sid;
  }

  function static_form_text() {
    return t('Term homepage');
  }

  function is_entry_form($form_id) {
    return $form_id == 'taxonomy_form_term';
  }

  function get_content_subdomain($nid) {
    static $subdomains;
    
    if (!isset($subdomains[$nid])) {
      // Get first group_nid
      $vid = variable_get('subdomain_vocab', '');
      $sid = db_query("SELECT tn.tid
        FROM {taxonomy_index} tn
        INNER JOIN {node} n on tn.nid = n.nid
        INNER JOIN {taxonomy_term_data} td on tn.tid = td.tid
        WHERE td.vid = :vid AND n.nid = :nid AND n.type NOT IN (:types)", 
        array(':vid' => $vid, ':nid' => $nid, ':types' => _subdomain_filtered_content_types()))
      ->fetchField();
      $subdomains[$nid] = subdomain_get_subdomain($sid);
    }
    return $subdomains[$nid];
  }

  function entry_form_alter(&$form, $form_id) {
    global $user;
    if ( $form['#vocabulary']->vid == variable_get('subdomain_vocab', 0) &&
         !($form['delete']['#type'] == 'value' && $form['delete']['#value'] == TRUE) ) {
      $default = subdomain_get_subdomain($form['tid']['#value']);
      if (empty($default) && user_access('create subdomains') || !empty($default) && user_access('edit subdomains')) {
        $form['subdomain_exists'] = array(
          '#type' => 'value',
          '#value' => !empty($default),
        );

        $form['#validate'][] = '_subdomain_validate_subdomain';

        if ($this->source == 'custom') {
          $form['identification']['name']['#weight'] = -5.1;

          $editable = user_access('edit subdomains');
          $form['identification']['subdomain'] = _subdomain_get_custom_form_field($default, $editable, $user->uid == 1);
        }
        _subdomain_add_js_validation($form['tid']['#value']);
      }
    }
  }

  function save($term) {
    if ($term->vid == variable_get('subdomain_vocab', 0)) {
      $this->save_record($term->tid, $term->{$this->form_field});
    }
  }

  function delete($term) {
    if ($term['vid'] == variable_get('subdomain_vocab', 0)) {
      $this->delete_record($term['tid']);
    }
  }

  function url_alter_helper(&$subdomain, &$path, $original_path, $outbound = TRUE) {
    if ($tid = _subdomain_id_from_path('tid', $original_path)) {
      $subdomain = subdomain_get_subdomain($tid);
      if (!empty($subdomain) && $path == $this->base_path($tid) && $outbound) {
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
    $vid = variable_get('subdomain_vocab', 0);
    $result = db_query("SELECT tid, name
      FROM {taxonomy_term_data} t LEFT JOIN {subdomain} s on t.tid = s.sid
      WHERE t.vid = :vid AND s.sid IS NULL", array(':vid' => $vid));
    $count = 0;
    foreach ($result as $row) {
      $this->save_record($row->tid, $row->name, TRUE);
      $count += 1;
    }
    drupal_set_message(format_plural($count, '1 new subdomain created.', '@count new subdomains created.'));
  }
}