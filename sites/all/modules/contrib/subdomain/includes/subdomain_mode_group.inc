<?php

/**
 * @file
 * Subdomain engine OG mode subclass
 */

/**
 * Description of subdomain_group
 *
 * @author setvik
 */
class subdomain_mode_group extends subdomain_engine {

  function type() {
    return 'node';
  }

  function base_path($sid) {
    return 'node/' . $sid;
  }

  function static_form_text() {
    return t('Group homepage');
  }

  function default_form_field() {
    return 'title';
  }

  function get_content_subdomain($nid) {
    static $subdomains;
    if (!isset($subdomains[$nid])) {
      $sid = db_query("SELECT gid from og_membership where field_name = 'og_group_ref' AND etid = :nid LIMIT 1",
        array(':nid' => $nid))->fetchField();
      $subdomains[$nid] = subdomain_get_subdomain($sid);
    }
    return $subdomains[$nid];
  }

  function is_entry_form($form_id) {
    // TODO update to select correct og group node type if multiple
    if (substr($form_id, -10) == '_node_form') {
      return og_is_group_type('node', substr($form_id, 0, -10));
    }
    return FALSE;
  }

  function entry_form_alter(&$form, $form_id) {
    $nid = isset($form['#node']->nid) ? $form['#node']->nid : NULL;
    $default = subdomain_get_subdomain($nid);
    $form['subdomain_exists'] = array(
      '#type' => 'value',
      '#value' => !empty($default),
    );

    $form['#validate'][] = '_subdomain_validate_subdomain';

    if ($this->source == 'custom') {
      $editable = user_access('edit subdomains');
      $form['subdomain'] = _subdomain_get_custom_form_field($default, $editable);
    }

    _subdomain_add_js_validation($nid);
  }

  function save($node) {
    if (og_is_group_type('node', $node->type)) {
      $this->save_record($node->nid, $node->{$this->form_field});

      // URL cache clear needed b/c url is cached before subdomain is saved resulting in skewed redirect: http://subdomain.example.com/node/xxxx instead of http://subdomain.example.com/
      subdomain_url_outbound_cache_clear();
    }
  }

  function delete($node) {
    if (og_is_group_type('node', $node->type)) {
      $this->delete_record($node->nid);
    }
  }

  function url_alter_helper(&$subdomain, &$path, $original_path, $outbound = TRUE) {
    if ($nid = _subdomain_id_from_path('nid', $original_path)) {
      $subdomain = subdomain_get_subdomain($nid);
      if (empty($subdomain)) {
        $subdomain = subdomain()->get_content_subdomain($nid);
      }
      elseif ($path == $this->base_path($nid) && $outbound) {
        $path = '';
      }
    }
  }

  /**
   * Bulk generate subdomains from group name
   */
  function bulk_generate() {
    $result = db_query("SELECT n.nid, n.uid, title
      FROM {node} n INNER JOIN {og} g ON n.nid = g.etid AND g.entity_type = 'node' LEFT JOIN {subdomain} s on n.nid = s.sid
      WHERE s.sid IS NULL AND n.type NOT IN (:types)", array(':types' => _subdomain_filtered_content_types()));
    $count = 0;
    foreach ($result as $row) {
      $account = user_load($row->uid);
      if (user_access('create subdomains', $account)) {
        $this->save_record($row->nid, $row->title, TRUE);
        $count += 1;
      }
    }
    drupal_set_message(format_plural($count, '1 new subdomain created.', '@count new subdomains created.'));
  }
}
