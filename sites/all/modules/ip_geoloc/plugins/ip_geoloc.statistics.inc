<?php


/**
 * @file
 * Capture IP Geoloc statistics in the access log.
 */

/**
 * Implements hook_better_statistics_fields().
 */
function ip_geoloc_better_statistics_fields() {
  $fields = array();

  // Pass all user-facing strings through t(), but always use English when first
  // declaring fields. They will be run through t() normally on output.
  $en = array('langcode' => 'en');

  $fields['geoloc_latitude'] = array(
    'schema' => array(
      'description' => 'Latitude',
    ),
    'views_field' => array(
      'title' => t('Latitude', array(), $en),
      'help' => t('Geographic latitude.', array(), $en),
    ),
  );

  $fields['geoloc_longitude'] = array(
    'schema' => array(
      'description' => 'Longitude',
    ),
    'views_field' => array(
      'title' => t('Longitude', array(), $en),
      'help' => t('Geographic longitude.', array(), $en),
    ),
  );

  $fields['geoloc_country'] = array(
    'schema' => array(
      'description' => 'Country',
    ),
    'views_field' => array(
      'title' => t('Country', array(), $en),
      'help' => t('Country.', array(), $en),
    ),
  );

  $fields['geoloc_country_code'] = array(
    'schema' => array(
      'description' => 'ISO 3166 2-Character Country Code',
    ),
    'views_field' => array(
      'title' => t('Country code', array(), $en),
      'help' => t('ISO 3166 2-Character Country Code.', array(), $en),
    ),
  );

  $fields['geoloc_region'] = array(
    'schema' => array(
      'description' => 'Region',
    ),
    'views_field' => array(
      'title' => t('Region', array(), $en),
      'help' => t('Region.', array(), $en),
    ),
  );

  $fields['geoloc_region_code'] = array(
    'schema' => array(
      'description' => '2-Character Region Code',
    ),
    'views_field' => array(
      'title' => t('Region code', array(), $en),
      'help' => t('2-Character Region Code.', array(), $en),
    ),
  );

  $fields['geoloc_city'] = array(
    'schema' => array(
      'description' => 'City',
    ),
    'views_field' => array(
      'title' => t('City', array(), $en),
      'help' => t('City.', array(), $en),
    ),
  );

  $fields['geoloc_postal_code'] = array(
    'schema' => array(
      'description' => 'Post code',
    ),
    'views_field' => array(
      'title' => t('Post code', array(), $en),
      'help' => t('Post code.', array(), $en),
    ),
  );

  $fields['geoloc_locality'] = array(
    'schema' => array(
      'description' => 'Suburb',
    ),
    'views_field' => array(
      'title' => t('Suburb', array(), $en),
      'help' => t('Suburb.', array(), $en),
    ),
  );

  $fields['geoloc_street'] = array(
    'schema' => array(
      'description' => 'Street',
    ),
    'views_field' => array(
      'title' => t('Street', array(), $en),
      'help' => t('Street.', array(), $en),
    ),
  );

  $fields['geoloc_street_number'] = array(
    'schema' => array(
      'description' => 'Street number',
    ),
    'views_field' => array(
      'title' => t('Street number', array(), $en),
      'help' => t('Street number.', array(), $en),
    ),
  );

  $fields['geoloc_admin_area_level_1'] = array(
    'schema' => array(
      'description' => 'State or province',
    ),
    'views_field' => array(
      'title' => t('State or province', array(), $en),
      'help' => t('State or province.', array(), $en),
    ),
  );

  $fields['geoloc_admin_area_level_2'] = array(
    'schema' => array(
      'description' => 'Area level 2',
    ),
    'views_field' => array(
      'title' => t('Area level 2', array(), $en),
      'help' => t('Area level 2.', array(), $en),
    ),
  );

  $fields['geoloc_admin_area_level_3'] = array(
    'schema' => array(
      'description' => 'Area level 3',
    ),
    'views_field' => array(
      'title' => t('Area level 3', array(), $en),
      'help' => t('Area level 3.', array(), $en),
    ),
  );

  $fields['geoloc_formatted_address'] = array(
    'schema' => array(
      'description' => 'Address',
    ),
    'views_field' => array(
      'title' => t('Address', array(), $en),
      'help' => t('Address.', array(), $en),
    ),
  );

  $fields['geoloc_provider'] = array(
    'schema' => array(
      'description' => 'Provider',
    ),
    'views_field' => array(
      'title' => t('Provider', array(), $en),
      'help' => t('Provider.', array(), $en),
    ),
  );

  $fields['geoloc_accuracy'] = array(
    'schema' => array(
      'description' => 'Accuracy',
    ),
    'views_field' => array(
      'title' => t('Accuracy', array(), $en),
      'help' => t('Accuracy.', array(), $en),
    ),
  );

  // Cycles through definitions to update schema type/size/legth.
  foreach ($fields as $field_name => &$field) {
    $field['callback'] = '_ip_geoloc_get_field_value';
    $field['schema']['not null'] = FALSE;
    list($ip_geoloc_key, $type, $size) = _ip_geoloc_key_map($field_name);
    switch ($type) {
      case 'boolean':
        $field['schema']['type'] = 'int';
        $field['schema']['size'] = 'tiny';
        break;

      case 'string':
        if ($size) {
          $field['schema']['type'] = 'varchar';
          $field['schema']['length'] = $size;
        }
        else {
          $field['schema']['type'] = 'text';
        }
        break;

      case 'float':
      case 'int':
        $field['schema']['type'] = $type;
        if ($size) {
          $field['schema']['size'] = $size;
        }
        break;

    }
  }
  return $fields;
}

/**
 * Helper function to map db fields to array keys.
 *
 * Maps accesslog db fields as exposed by hook_better_statistics_fields() to
 * the corresponding array key of the array returned by ip_geoloc_get_visitor_location()
 * inclusive of expected type and size, and vice-versa.
 *
 * @param string $db_field_name
 *   The name of the db field for which to return the corresponding array
 *   key.
 * @param string $ip_geoloc_key
 *   The name of the array key for which to return the corresponding db
 *   field.
 *
 * @return mixed
 *   Either way, the an array containing ip_geoloc array key, type and size, or
 *   a string with the db field name. If no arguments specified, return the
 *   entire mapping array.
 */
function _ip_geoloc_key_map($db_field_name = NULL, $ip_geoloc_key = NULL) {
  static $keys;
  if (!isset($keys)) {
    $keys = array(
      'geoloc_latitude'               => array('latitude', 'float', 'big'),
      'geoloc_longitude'              => array('longitude', 'float', 'big'),
      'geoloc_country'                => array('country', 'string' , 64),
      'geoloc_country_code'           => array('country_code', 'string', 3),
      'geoloc_region'                 => array('region', 'string' , 64),
      'geoloc_region_code'            => array('region_code', 'string', 3),
      'geoloc_city'                   => array('city', 'string' , 64),
      'geoloc_postal_code'            => array('postal_code', 'string' , 12),
      'geoloc_locality'               => array('locality', 'string' , 64),
      'geoloc_street'                 => array('route', 'string' , 64),
      'geoloc_street_number'          => array('street_number', 'string' , 32),
      'geoloc_admin_area_level_1'     => array('administrative_area_level_1', 'string' , 64),
      'geoloc_admin_area_level_2'     => array('administrative_area_level_2', 'string' , 64),
      'geoloc_admin_area_level_3'     => array('administrative_area_level_3', 'string' , 64),
      'geoloc_formatted_address'      => array('formatted_address', 'string' , 128),
      'geoloc_provider'               => array('provider', 'string' , 64),
      'geoloc_accuracy'               => array('accuracy', 'float' , 'big'),
    );
  }
  if ($db_field_name) {
    return isset($keys[$db_field_name]) ? $keys[$db_field_name] : NULL;
  }
  elseif ($ip_geoloc_key) {
    foreach ($keys as $key => $item) {
      if ($item[0] == $ip_geoloc_key) {
        return $key;
      }
    }
    return NULL;
  }
  else {
    return $keys;
  }
}

/**
 * Returns a value to be inserted into the accesslog based on a field name
 * provided. This handles Drupal Core's values as well as our own.
 *
 * @param $field
 *   The name of the field for which to return data.
 *
 * @return
 *   The data to be inserted into the accesslog for the provided field.
 */
function _ip_geoloc_get_field_value($field) {

  // Sanity check, is the required field mapped?
  if (!$ip_geoloc_item = _ip_geoloc_key_map($field)) {
    return NULL;
  }
  list($ip_geoloc_key, $type, $size) = $ip_geoloc_item;

  static $geoloc; // see [#1970180]

  if (!isset($geoloc)) {
    // If caching, the module will not be loaded. In such case,
    // ip_geoloc_get_visitor_location() will be undefined, and an alternative
    // method to retrieve data should be used.
    if (function_exists('ip_geoloc_get_visitor_location')) {
      $geoloc = ip_geoloc_get_visitor_location(); // uses internal cache
    }
    else {
      // Include common.inc.
      require_once DRUPAL_ROOT . '/includes/common.inc';
      $list = system_list('module_enabled');
      // Include 'IP Geolocation Views and Maps' module file.
      require_once DRUPAL_ROOT . '/' . $list['ip_geoloc']->filename;
      // If 'Smart IP' module is enabled and set up as backup, include its
      // module file.
      if (variable_get('ip_geoloc_smart_ip_as_backup', TRUE) && isset($list['smart_ip'])) {
        require_once DRUPAL_ROOT . '/' . $list['smart_ip']->filename;
      }
      // If 'GeoIP API' module is enabled, include its module file.
      if (isset($list['geoip'])) {
        require_once DRUPAL_ROOT . '/' . $list['geoip']->filename;
      }
      // Retrieve geolocation.
      // ip_geoloc_get_location_by_ip() will either fetch a stored geolocation
      // from {ip_geoloc}, or manage conversation with backup modules to
      // retrieve the information and store back to {ip_geoloc}.
      $geoloc = ip_geoloc_get_location_by_ip(ip_address(), $resample = FALSE, $store = TRUE, $reverse_geocode = FALSE);
      if (!empty($geoloc) and (!isset($geoloc['provider']) or empty($geoloc['provider']))) {
        $geoloc['provider'] = 'ip_geoloc';
      }
    }
  }
  // No gelocation data available, return NULL.
  if (empty($geoloc) || !isset($geoloc[$ip_geoloc_key])) {
    return NULL;
  }

  // Return data, ensuring right type and size.
  switch ($type) {
    case 'string':
      if ($size) {
        return drupal_substr($geoloc[$ip_geoloc_key], 0, $size);
      }
      else {
        return (string) $geoloc[$ip_geoloc_key];
      }
    case 'boolean':
      return empty($geoloc[$ip_geoloc_key]) ? 0 : 1;
    case 'float':
    case 'int':
      settype($geoloc[$ip_geoloc_key], $type);
      return $geoloc[$ip_geoloc_key];
    default:
      return NULL;
  }
}

/**
 * Backfills position information to past {accesslog} entries.
 *
 * Given that location information is received asynchronously, here
 * we 'backfill' such information to all accesslog entries since when
 * the location request was initiated.
 *
 * @param integer $from_timestamp
 *   The timestamp of the location request.
 * @param array $location
 *   The location infromation array.
 */
function _ip_geoloc_statistics_backfill($from_timestamp, $location) {

  // If Better Statistics is set to skip logging to the database, then exit now.
  if (!variable_get('statistics_log_to_db', TRUE)) {
    return;
  }

  // Fetch the fields defined in {accesslog}.
  $accesslog_fields = variable_get('better_statistics_fields', better_statistics_get_default_fields());

  // Loops through location keys to check which ones need to be updated.
  $fields = array();
  foreach ($location as $key => $value) {
    $field_name = _ip_geoloc_key_map(NULL, $key);
    if ($field_name and isset($accesslog_fields[$field_name])) {
      $fields[$field_name] = $value;
    }
  }

  // Loops through fields not in location to blank them.
  $ip_geoloc_fields = array_keys(_ip_geoloc_key_map());
  foreach ($ip_geoloc_fields as $i => $field_name) {
    if (!isset($fields[$field_name]) and isset($accesslog_fields[$field_name])) {
      $fields[$field_name] = NULL;
    }
  }

  // Run update. Take one extra second to from_timestamp to ensure initial hit
  // requesting location is included.
  if (!empty($fields)) {
    try {
      $num_updated = db_update('accesslog')
        ->fields($fields)
        ->condition('timestamp', $from_timestamp - 1, '>=')
        ->condition('sid', session_id(), '=')
        ->execute();
    }
    catch (Exception $e) {
      watchdog('IP Geolocation', 'There was an error updating statistics data:<br/>@error', array(
        '@error' => $e->getMessage()), WATCHDOG_ERROR);
    }
  }
}
