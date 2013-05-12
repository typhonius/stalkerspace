<?php

/**
 * @file
 * Subdomain admin settings
 */

function subdomain_admin_settings($form, &$form_state) {
  drupal_add_css(drupal_get_path('module', 'subdomain') . '/subdomain.css');
  drupal_add_js(drupal_get_path('module', 'subdomain') . '/subdomain.js', 'file');
  
  /* Add Admin screen JS settings */
  $data = array(
    'user' => t('User name'),
    'contenttype' => t('Content type name'),
    'term' => t('Term name'),
    'group' => t('Group name'),
  );
  drupal_add_js(array('subdomain' => array('sourceOptionText' => $data)), 'setting');
  
  // We always have user mode
  $modes['user'] = t('User subdomains');
  
  // Add conditional modes
  if (module_exists('views')) {
    // Content Type mode requires a view homepage, so ensure views module exists :-)
    $modes['contenttype'] = t('Content type subdomains');
  }
  if (module_exists('taxonomy')) {
    $modes['term'] = t('Taxonomy subdomains');
  }
  if (module_exists('og')) {
    $modes['group'] = t('Organic group subdomains');
  }
  
  $form['mode'] = array(
    '#type' => 'fieldset',
  );
  $form['mode']['subdomain_mode'] = array(
    '#type' => 'select',
    '#title' => t('Mode'),
    '#options' => $modes,
    '#default_value' => variable_get('subdomain_mode', 'user'),
  );
  
  $form['mode']['subdomain_source'] = array(
    '#type' => 'select',
    '#title' => t('Source for subdomain text'),
    '#options' => array(
      'default' => t('Automatic'),
      'custom' => t('User can specify a custom subdomain'),
    ),
    '#default_value' => variable_get('subdomain_source', 'default'),
  );
  
  if (module_exists('taxonomy')) {
    $vocabs = taxonomy_get_vocabularies();
    $vocab_options = array();
    if (is_array($vocabs)) {
      foreach ($vocabs as $vid => $vocab) {
        $vocab_options[$vid] = $vocab->name;
      }
    }
    
    if (count($vocab_options)) {
      $form['mode']['subdomain_vocab'] = array(
        '#type' => 'select',
        '#title' => t('Select a vocabulary'),
        '#description' => t('Terms in the selected vocabulary will have their own subdomain.'),
        '#options' => $vocab_options,
        '#default_value' => variable_get('subdomain_vocab', NULL),
        '#states' => array(
          // Only show vocab selection when in term mode
          'visible' => array(
            ':input[name="subdomain_mode"]' => array('value' => 'term'),
          ),
        ),
      );
    }
    else {
      $form['mode']['subdomain_vocab'] = array(
        '#type' => 'item',
        '#title' => t('Taxonomy vocabulary'),
        '#description' => t('<span class="error">No non-tag vocabularies found.<br />Create one before configuring taxonomy subdomains.</span>'),
        '#prefix' => '<div id="edit-subdomain-vocab-wrapper">',
        '#suffix' => '</div>',
        '#states' => array(
          // Only show vocab msg when in term mode
          'visible' => array(
            ':input[name="subdomain_mode"]' => array('value' => 'term'),
          ),
        ),
      );
    }
  }
  
  if (module_exists('views')) {
    $views = views_get_all_views();
    $view_options = array();
    foreach ($views as $name => $view) {
      if (!isset($view->disabled) || !$view->disabled) {
        $view_options[$name] = $name;
      }
    }
    
    $form['mode']['subdomain_home'] = array(
      '#type' => 'fieldset',
      '#title' => t('Subdomain homepage'),
    );
    $form['mode']['subdomain_home']['subdomain_home_default'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use default'),
      '#default_value' => variable_get('subdomain_home_default', TRUE),
      '#states' => array(
        'unchecked' => array(
          ':input[name="subdomain_mode"]' => array('value' => 'contenttype'),
        ),
        // Hide default option when in contenttype mode
        'invisible' => array(
          ':input[name="subdomain_mode"]' => array('value' => 'contenttype'),
        ),
      ),
    );
    if (count($view_options)) {
      $form['mode']['subdomain_home']['subdomain_home_view'] = array(
        '#type' => 'select',
        '#title' => t('Use a view'),
        '#options' => $view_options,
        '#default_value' => variable_get('subdomain_home_view', ''),
        '#states' => array(
          'invisible' => array(
            ':input[name="subdomain_home_default"]' => array('checked' => TRUE),
          ),
          // Make view selection required when in contenttype mode
          'required' => array(
            ':input[name="subdomain_mode"]' => array('value' => 'contenttype'),
          ),
        ),
      );
    }
    else {
      $form['mode']['subdomain_home_view'] = array(
        '#type' => 'item',
        '#title' => t('Homepage view'),
        '#description' => t('<span class="error">No enabled views found.<br />Create a view before selecting a "custom view" for your subdomain homepages.</span>'),
        '#prefix' => '<div id="edit-subdomain-home-view-wrapper">',
        '#suffix' => '</div>',
      );
    }
    
  }
  
  $base = subdomain_get_domain();
  
  $form['subdomain_mismatch_response'] = array(
    '#type' => 'select',
    '#title' => t("Subdomain mismatch response"),
    '#description' => t('How should we handle content that is being accessed on the wrong subdomain.'),
    '#options' => array(
      'none'  => t('- Do Nothing -'),
      '301'   => t('301 Moved Permanently'),
      '302'   => t('302 Moved Temporarily'),
      '403'   => t('403 Forbidden'),
      '404'   => t('404 Not Found'),
    ),
    '#default_value' => variable_get('subdomain_mismatch_response', '404'),
  );
  
  $form['subdomain_prepend_www'] = array(
    '#type' => 'checkbox',
    '#title' => t("Prepend 'www' to non-subdomain URLs (i.e. use <strong>http://www.$base</strong> instead of <strong>http://$base</strong>)"),
    '#default_value' => variable_get('subdomain_prepend_www', TRUE),
  );
  
  $form['#submit'][] = 'subdomain_admin_settings_submit';
  
  // Ensure server is setup to properly handle subdomains
  _subdomain_validate_dns();
  _subdomain_validate_cookie_domain();
  _subdomain_validate_webserver_config();
  
  return system_settings_form($form);
}

function subdomain_admin_settings_validate($form, &$form_state) {
  // Extract form values into simple variables
  foreach (array('mode', 'vocab', 'source', 'home_default', 'home_view') as $val) {
    if (isset($form_state['values']['subdomain_' . $val])) {
      ${$val} = $form_state['values']['subdomain_' . $val];
    }
  }
  
  if ($mode == 'term' && empty($vocab)) {
    form_set_error('subdomain_mode', t('You must create at least one non-tag vocabulary before configuring taxonomy subdomains.'));
  }
  
  if (isset($home_default) && !$home_default && empty($home_view)) {
    form_set_error('subdomain_home', t('You must create or enable at least one view before selecting a "Custom view" for your subdomain homepages.'));
  }
  elseif (isset($home_default) && !$home_default) {
    // Check for presence of appropriate argument
    $view = views_get_view($home_view);
    $args = $view->display['default']->display_options['arguments'];
    
    $first_arg = empty($args) ? NULL : array_shift(array_keys($args));
    
    if (empty($first_arg)) {
      form_set_error('subdomain_view', t('The selected view has no arguments. Add a uid/nid/tid argument to your view to filter your content for the active subdomain.'));
    }
    elseif ($mode == 'term' && $first_arg != 'tid'
    || $mode == 'group' && !in_array($first_arg, array('nid', 'group_nid'))
    || $mode == 'user' && $first_arg != 'uid') {
      drupal_set_message(t("The selected view's first argument was @arg. This may not work as expected.", array('@arg' => $first_arg)), 'warning');
    }
  }
}

function subdomain_admin_settings_submit($form, &$form_state) {
  // If mode has changed, wipe subdomain tables
  // TODO: need user to confirm this...
  
  if ($form_state['values']['subdomain_mode'] != $form['subdomain_mode']['#default_value']) {
    db_query("DELETE FROM {subdomain};");
    if ($form_state['values']['subdomain_mode'] != 'contenttype') {
      variable_del('subdomain_contenttypes');
    }
  }
  
  // _subdomain_clear_caches();
}

function subdomain_filters_form() {
  $form['subdomain_filter_contenttypes'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Exclude the following content types from subdomains'),
    '#description' => t('Nodes from the selected types will not be placed on subdomains.'),
    '#options' => node_type_get_names(),
    '#default_value' => variable_get('subdomain_filter_contenttypes', array()),
  );
  
  $form['subdomain_reserved'] = array(
    '#type' => 'textarea',
    '#title' => t('Disallow the following subdomains'),
    '#rows' => 5,
    '#description' => t('Enter 1 subdomain per line'),
    '#default_value' => variable_get('subdomain_reserved', '')
  );
  
  $path_explanation = t(
    "Enter one path pattern per line. The string '%ID' is a placeholder for the ID we are looking for.
     To detect the user ID in the path 'user/1234/foo' you would need to add the pattern 'user/%ID/foo'."
  );
  
  $form['entities'] = array(
    '#type' => 'fieldset',
    '#title' => t('Entity IDs'),
    '#description' => t('Drupal will make a best effort to identify entity IDs in a path, but sometimes
                         this does not work. This is most notably an issue with views pages.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['entities']['file_path']['subdomain_nid_paths'] = array(
    '#type' => 'textarea',
    '#title' => t('Node IDs in paths'),
    '#default_value' => variable_get('subdomain_nid_paths', _subdomain_id_from_path_defaults('nid')),
    '#description' => $path_explanation,
  );
  $form['entities']['file_path']['subdomain_uid_paths'] = array(
    '#type' => 'textarea',
    '#title' => t('User IDs in paths'),
    '#default_value' => variable_get('subdomain_uid_paths', _subdomain_id_from_path_defaults('uid')),
    '#description' => $path_explanation,
  );
  $form['entities']['file_path']['subdomain_tid_paths'] = array(
    '#type' => 'textarea',
    '#title' => t('Taxonomy term IDs in paths'),
    '#default_value' => variable_get('subdomain_tid_paths', _subdomain_id_from_path_defaults('tid')),
    '#description' => $path_explanation,
  );
  
  $path_explanation = t(
    "Enter one URL pattern per line. The '*' character is a wildcard.<br />
     Example path patterns are <code>*.js</code> for all JavaScript files and
     <code>mytheme/*.css</code> for all CSS files in the <code>mytheme</code>
     directory."
  );
  
  $form['system'] = array(
    '#type' => 'fieldset',
    '#title' => t('System paths'),
    '#description' => t('Some files should be served from the same subdomain as the
                         current page. You can easily define those system paths here, either
                         by file path, Drupal path or by Drupal path for authenticated users.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  
  $form['system']['file_path'] = array(
    '#type' => 'fieldset',
    '#title' => t('File URL'),
    '#description' => t("Files that are marked to not be served from another subdomain
                         because of a match in the blacklist, can be overridden to be served from
                         another subdomain after all, if they have a match in the whitelist.<br />
                         <br />
                         <strong>All JavaScript files are blacklisted by default. This is necessary
                         to ensure a painless out-of-the-box experience.</strong>
                         <br />
                         <strong><u>Full explanation:</u></strong> it's necessary prevent any
                         possible cross-domain AJAX requests, which would violate the <a
                         href=\"http://en.wikipedia.org/wiki/Same_origin_policy\">same origin
                         policy</a> of browsers. Such violations potentially result in broken
                         functionality. Note that <u>even requests to another subdomain count as cross-domain
                         requests!</u> Usually JavaScript files will automatically be served from the same
                         subdomain. This feature is here for the rare occasions when this doesn't happen.
                         If you are looking to serve your JavaScript and CSS files from another subdomain, you
                         should take a look at the <a href=\"http://drupal.org/project/cdn\">CDN module</a>.<br />
                         <br />
                         You can opt-in to including JavaScript files by default
                         and then exclude problematic ones, but then you should carefully note which
                         JavaScript files perform AJAX requests. You can prevent all potential problems
                         by using <a href=\"http://en.wikipedia.org/wiki/JSON#JSONP\">JSONP</a>,
                         which is a work-around to allow for cross-domain AJAX
                         requests.</strong>"),
    '#collapsible' => TRUE,
  );
  $form['system']['file_path']['subdomain_system_file_path_blacklist'] = array(
    '#type' => 'textarea',
    '#title' => t('Blacklist'),
    '#default_value' => variable_get('subdomain_system_file_path_blacklist', SUBDOMAIN_SYSTEM_FILE_PATH_BLACKLIST_DEFAULT),
    '#description' => $path_explanation,
  );
  $form['system']['file_path']['subdomain_system_file_path_whitelist'] = array(
    '#type' => 'textarea',
    '#title' => t('Whitelist'),
    '#default_value' => variable_get('subdomain_system_file_path_whitelist', SUBDOMAIN_SYSTEM_FILE_PATH_WHITELIST_DEFAULT),
    '#description' => $path_explanation,
  );

  $form['system']['drupal_path'] = array(
    '#type' => 'fieldset',
    '#title' => t('Drupal path'),
    '#description' => t("Drupal paths entered in this blacklist will not contain any
                         links/files from another subdomain. This blacklist is applied for
                         <em>all</em> users."),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['system']['drupal_path']['subdomain_system_drupal_path_blacklist'] = array(
    '#type' => 'textarea',
    '#title' => t('Blacklist'),
    '#default_value' => variable_get('subdomain_system_drupal_path_blacklist', SUBDOMAIN_SYSTEM_DRUPAL_PATH_BLACKLIST_DEFAULT),
    '#description' => t("Enter one page per line as Drupal paths. The '*'
                         character is a wildcard. Example paths are %blog for
                         the blog page and %blog-wildcard for every personal
                         blog. %front is the front page.",
                         array(
                           '%blog'          => 'blog',
                           '%blog-wildcard' => 'blog/*',
                           '%front'         => '<front>',
                          )
     ),
  );
  
  $form['system']['auth_users'] = array(
    '#type' => 'fieldset',
    '#title' => t('Drupal path for authenticated users'),
    '#description' => t("Drupal paths entered in this blacklist will not contain
                         any links/files from another subdomain. This blacklist is applied for
                         <em>authenticated users only</em>."),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['system']['auth_users']['subdomain_system_auth_users_blacklist'] = array(
    '#type'          => 'textarea',
    '#title'         => t('Blacklist'),
    '#default_value' => variable_get('subdomain_system_auth_users_blacklist', SUBDOMAIN_SYSTEM_AUTH_USERS_BLACKLIST_DEFAULT),
    '#description'   => $path_explanation,
  );
  
  return system_settings_form($form);
}

function subdomain_tools() {
  return drupal_get_form('subdomain_tools_form');
}


function subdomain_tools_form($form, &$form_state) {
  static $bulk_generated = FALSE; // TODO : used b/c D7 seems to be executing this twice on form submit
  
  // TODO: We should be using the batch functions.
  
  $op = isset($_POST['op']) ? $_POST['op'] : '';
  if ($op == t('Generate subdomains')) {
    if (!$bulk_generated) {
      $bulk_generated = TRUE;
      subdomain()->bulk_generate();
    }
  }
  
  $form['subdomain_bulk_generate'] = array(
    '#type' => 'fieldset',
    '#title' => t('Bulk generate missing subdomains'),
    '#description' => t('Normally you wouldn\'t use this. Useful for testing or for auto-generating subdomains for an existing site.'),
  );
  
  $form['subdomain_bulk_generate']['subdomain_bulk_generate_button'] = array(
    '#type' => 'button',
    '#value' => t('Generate subdomains'),
  );
  return $form;
}

// TODO: probably don't need
function _subdomain_clear_caches() {
  // Flush all caches; no need to re-implement this.
  module_load_include('inc', 'system', 'system.admin');
  $form = $form_state = array();
  system_clear_cache_submit($form, $form_state);
}

function _subdomain_validate_dns($display_error = TRUE) {
  $domain = trim(subdomain_get_domain(), '.');
  $subdomain = SUBDOMAIN_TEST_SUBDOMAIN . '.' . $domain;
  
  $domain_ip = gethostbyname($domain);
  $subdomain_ip = gethostbyname($subdomain);
  
  if ($domain_ip != $subdomain_ip && $display_error) {
    drupal_set_message(t("<strong>Subdomain error:</strong> @domain and @subdomain did not resolve to the same IP address. Your DNS may be improperly configured and subdomains will likely not work.", array('@domain' => $domain, '@subdomain' => $subdomain)), 'error');
  }
  return $subdomain_ip == $domain_ip;
}

function _subdomain_validate_webserver_config() {
  if (_subdomain_validate_dns(FALSE)) {
    $domain = trim(subdomain_get_domain(), '.');
    
    $url = 'http://' . SUBDOMAIN_TEST_SUBDOMAIN . '.' . $domain . '/subdomain/test';
    
    if (drupal_http_request($url)->data != SUBDOMAIN_TEST_RESPONSE) {
      drupal_set_message(t("<strong>Subdomain error:</strong> @subdomain was not correctly routed to this site. Ensure your webserver is setup to correctly route wildcard subdomains to this site", array('@domain' => $domain, '@subdomain' => $subdomain)), 'error');
    }
  }
}

function _subdomain_validate_cookie_domain($display_error = TRUE) {
  $settings = file_get_contents('./' . conf_path() . '/settings.php');
  $settings = str_replace(chr(13) . chr(10), "\n", $settings);
  $settings = explode("\n", $settings);
  
  foreach ($settings as $setting) {
    if (strpos($setting, '$cookie_domain') !== FALSE) {
      // TODO: look at alternative to eval
      eval($setting);
    }
  }
  
  if (!isset($cookie_domain) && $display_error) {
    drupal_set_message(t('<strong>Subdomain error:</strong> The $cookie_domain variable in settings.php must be set or subdomains will not work.'), 'error');
  }
  return isset($cookie_domain);
}

function _subdomain_test_request() {
  // Turn devel module shutdown functions off if enabled.
  $GLOBALS['devel_shutdown'] = FALSE;
  
  echo SUBDOMAIN_TEST_RESPONSE;
  exit;
}
