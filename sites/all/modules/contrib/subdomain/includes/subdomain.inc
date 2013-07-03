<?php

/**
 * @file
 * Subdomain engine base class
 */

/**
 * Description of subdomain engine
 */
class subdomain_engine {
  var $mode;
  var $source;
  var $home_default;
  var $home_view;
  var $form_field;

  // Constructor
  function subdomain_engine() {
    $this->mode = variable_get('subdomain_mode', 'user');
    $this->source = variable_get('subdomain_source', 'default');
    $this->home_default = variable_get('subdomain_home_default', TRUE);
    $this->home_view = variable_get('subdomain_home_view', '');

    if ($this->source == 'default') {
      $this->form_field = $this->default_form_field();
    }
    else {
      $this->form_field = 'subdomain';
    }
  }
  
  function type() {
    return '';
  }
  
  function base_path($sid) {
    return '';
  }
  
  function get_content_subdomain($nid) {
    return NULL;
  }
  
  function url_alter_helper(&$subdomain, &$path, $original_path, $outbound = TRUE) {
  }
  
  function is_entry_form($form_id) {
    return FALSE;
  }
  
  function entry_form_alter(&$form, $form_id) {
  }
  
  function static_form_text() {
    return '';
  }

  function default_form_field() {
    return 'name';
  }

  function get_homepage_path($sid) {
    if ($this->home_default) {
      return subdomain()->base_path($sid);
    }
    else {
      return "subdomain/homepage/$sid";
    }
  }

  function data_handler($op, $requireMode, $object) {
    if ($requireMode == $this->mode) {
      switch ($op) {
        case 'insert':
          if (user_access('create subdomains')) {
            $this->save($object);
          }
          break;
        case 'update':
          if (user_access('edit subdomains')) {
            $this->save($object);
          }
          break;
        case 'delete':
          $this->delete($object);
          break;
      }
    }
  }

  /**
   * Prepares subdomain for saving
   */
  function clean($raw) {
    $clean = $raw;
    
    // Try to use the transliteration module if available.
    if (function_exists('transliteration_get')) {
      $clean = transliteration_get($clean, '-', language_default('language'));
    }
    
    // Replace spaces with dashes, and make sure we don't have multiple dashes in a row.
    $clean = str_replace(' ', '-', $clean);
    $clean = preg_replace('/--+/i', '-', $clean);
    
    /*
    Hostname spec:
    The original specification of hostnames in RFC 952, mandated that labels could not start with a digit or with a hyphen, and must not
    end with a hyphen. However, a subsequent specification (RFC 1123) permitted hostname labels to start with digits.
    */
    
    // Remove any remaining invalid characters
    $clean = preg_replace("/[^a-z0-9-]/i", "", $clean);
    
    // Remove any trailing dash
    $clean = preg_replace("/-$/i", "", $clean);
    
    // Are we going to allow the subdomain to start with a numeral (RCC 1123)?
    if (!variable_get('subdomain_allow_rfc1123', FALSE)) {
      // Clean until nothing is replaced.
      $dirty = TRUE;
      while ($dirty) {
        // Remove leading digits
        $clean = preg_replace("/^[0-9]+/i", "", $clean, -1, $dirty);
        
        // Remove any exposed leading dash
        $clean = preg_replace("/^-/i", "", $clean);
      }
    }
    
    // Make sure we return a lowercase subdomain
    $clean = strtolower($clean);
    
    return $clean;
  }

  function exists($value) {
    $sid = subdomain_get_sid($value);
    return !empty($sid);
  }

  function save_record($id, $raw) {
    $row->sid = $id;
    $row->subdomain = $this->clean($raw);
    if ($row->subdomain) {
      db_merge('subdomain')
        ->key(array('sid' => $row->sid))
        ->fields(array(
            'subdomain' => $row->subdomain,
        ))
        ->execute();
    }
    else {
      // Delete subdomain if blank (site admins can enter blank subdomains)
      $this->delete_record($row->sid);
    }
  }

  function delete_record($sid) {
    db_query('DELETE FROM {subdomain} WHERE sid = :sid', array(':sid' => $sid));
  }
}
