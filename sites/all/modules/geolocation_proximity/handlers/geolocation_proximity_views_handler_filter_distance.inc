<?php
/**
 * @file
 * Definition of geolocation_proximity_views_handler_filter_distance.
 */

/**
 * General proximity filter for location latitude/longitude.
 */
class geolocation_proximity_views_handler_filter_distance extends views_handler_filter {

  /**
   * init the handler with necessary data.
   */
  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->field_alias = $this->options['id'] . '_filter';
  }

  /**
   * views_handler_filter::option_definition
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['operator'] = array('default' => '<');
    $options['value']['default'] = array(
      'latitude' => NULL,
      'longitude' => NULL,
      'search_distance' => 100,
      'search_units' => 'kilometers',
    );
    return $options;
  }

  /**
   * Display the filter on the administrative summary
   */
  function admin_summary() {
    return FALSE;
  }

  /**
   * Shortcut to display the value form.
   */
  function value_form(&$form, &$form_state) {

    $form['value'] = array(
      '#tree' => TRUE,
    );

    $input_type = !empty($this->value['type']) ? $this->value['type'] : 'direct_input';

    if (empty($form_state['exposed'])) {
      if (module_exists('geolocation_googlemaps') && $this->options['exposed']) {
        $form['value']['type'] = array(
          '#type' => 'radios',
          '#title' => t('Input type'),
          '#options' => array(
            'direct_input' => t('Direct input of latitude, longitude and distance.'),
            'googlemap' => t('Geolocation Googlemaps enhanced widget.'),
          ),
          '#default_value' => $input_type,
        );
      }
      else {
        $form['value']['type'] = array(
          '#type' => 'value',
         '#value' => 'direct_input',
        );
      }
    }

    if (empty($form_state['exposed']) || (!empty($form_state['exposed']) && $input_type == 'direct_input')) {
      $form['value']['latitude'] = array(
        '#type' => 'textfield',
        '#title' => t('Latitude'),
        '#default_value' => $this->value['latitude'],
      );
      $form['value']['longitude'] = array(
        '#type' => 'textfield',
        '#title' => t('Longitude'),
        '#default_value' => $this->value['longitude'],
      );
    }

    if (!empty($form_state['exposed']) && $input_type == 'googlemap') {

      if (empty($this->view->live_preview)) {

        $id = $form_state['view']->name . '-' . $form_state['view']->current_display;

        $form['value']['googlemap'] = array(
          '#type' => 'container',
          '#attributes' => array(
            'class' => array('field-widget-geolocation-googlemap'),
          ),
        );

        $form['value']['googlemap']['address'] = array(
          '#type' => 'item',
          '#title' => t('Location'),
          '#prefix' => '<div id="geolocation-address-' . $id . '" class="geolocation-address">',
          '#suffix' => '</div>',
          '#required' => FALSE,
        );
        $form['value']['googlemap']['address']['field'] = array(
          '#type' => 'textfield',
          '#maxlength' => 120,
        );
        $form['value']['googlemap']['address']['geocode'] = array(
          '#prefix' => '<span id="geolocation-address-geocode-' . $id . '" class="geolocation-address-geocode">',
          '#suffix' => '</span>',
          '#markup' => t('Get location'),
        );
        $form['value']['googlemap']['help'] = array(
          '#prefix' => '<div id="geolocation-help-' . $id . '" class="geolocation-help">',
          '#suffix' => '</div>',
          '#markup' => t('Enter an address / location in the textfield or click on the map to set a marker'),
        );
        $form['value']['googlemap']['googlemap'] = array(
          '#prefix' => '<div id="geolocation-map-' . $id . '" class="geolocation-map">',
          '#suffix' => '</div>',
        );
        // Presentational item.
        $form['value']['googlemap']['latitem'] = array(
          '#type' => 'item',
          '#title' => t('Latitude:'),
          '#prefix' => '<div id="geolocation-lat-item-' . $id . '" class="geolocation-lat-item">',
          '#suffix' => '</div>',
          '#markup' => '<span class="geolocation-lat-item-value">' . $this->value['latitude'] . '</span>',
          '#required' => FALSE,
        );
        $form['value']['googlemap']['lat'] = array(
          '#type' => 'hidden',
          '#prefix' => '<div id="geolocation-lat-' . $id . '" class="geolocation-lat">',
          '#suffix' => '</div>',
          '#default_value' => $this->value['latitude'],
        );
        // Presentational item.
        $form['value']['googlemap']['lngitem'] = array(
          '#type' => 'item',
          '#title' => t('Longitude:'),
          '#prefix' => '<div id="geolocation-lng-item-' . $id . '" class="geolocation-lng-item">',
          '#suffix' => '</div>',
          '#markup' => '<span class="geolocation-lat-item-value">' . $this->value['longitude'] . '</span>',
          '#required' => FALSE,
        );
        $form['value']['googlemap']['lng'] = array(
          '#type' => 'hidden',
          '#prefix' => '<div id="geolocation-lng-' . $id . '" class="geolocation-lng">',
          '#suffix' => '</div>',
          '#default_value' => $this->value['longitude'],
        );
        $form['value']['googlemap']['remove'] = array(
          '#prefix' => '<div id="geolocation-remove-' . $id . '" class="geolocation-remove"><span>',
          '#suffix' => '</span></div>',
          '#markup' => t('Remove'),
        );

        // Attach CSS and JS files via FAPI '#attached'.
        $form['value']['googlemap']['googlemap']['#attached']['css'][] = drupal_get_path('module', 'geolocation_googlemaps') . '/geolocation_googlemaps.css';
        $form['value']['googlemap']['googlemap']['#attached']['js'][] = array(
          'data' => '//maps.google.com/maps/api/js?sensor=false',
          'type' => 'external',
        );
        $form['value']['googlemap']['googlemap']['#attached']['js'][] = array(
          'data' => '//www.google.com/jsapi',
          'type' => 'external',
        );
        $form['value']['googlemap']['googlemap']['#attached']['js'][] = array(
          'data' => drupal_get_path('module', 'geolocation_googlemaps') . '/geolocation_googlemaps_widget.js',
          'type' => 'file',
          'scope' => 'footer',
        );

        // Make defaults available as javascript settings. In JS files use:
        // Drupal.settings.mapDefaults.lat
        $map_defaults = array(
          $id => array(
            'lat' => $this->value['latitude'],
            'lng' => $this->value['longitude'],
          ),
        );
        $data = array(
          'defaults' => $map_defaults,
          'settings' => array(
            'marker_draggable' => TRUE,
            'scrollwheel' => FALSE,
          ),
        );
        $form['value']['googlemap']['googlemap']['#attached']['js'][] = array(
          'data' => array('geolocation' => $data),
          'type' => 'setting',
        );
      }
      else {
        $form['value']['googlemap'] = array(
          '#markup' => t('Geolocation proximity Googlemaps exposed filter is not compatible with live preview.'),
        );
        $form['value']['latitude'] = $form['value']['longitude'] = array(
          '#type' => 'value',
          '#value' => "",
        );
      }
    }

    $form['value']['search_distance'] = array(
      '#type' => 'textfield',
      '#title' => t('Max distance (km)'),
      '#default_value' => $this->value['search_distance'],
      '#required' => TRUE,
    );
  }

  /**
   * Validate the options form.
   */
  function value_validate($form, &$form_state) {
    $this->latlng_validate($form['value'], $form_state['values']['options']['value']);
  }

  function exposed_validate(&$form, &$form_state) {
    $this->latlng_validate($form[$this->options['id']], $form_state['values'][$this->options['id']]);
  }

  /**
   * Validate the latitude and longitude values
   */
  function latlng_validate(&$elements, &$values) {

    if (!empty($values['googlemap'])) {
      $values['latitude'] = $values['googlemap']['lat'];
      $values['longitude'] = $values['googlemap']['lng'];
    }

    if ($values['latitude'] !== "") {
      switch (TRUE) {
        case !is_numeric($values['latitude']):
          form_error($elements['latitude'], t('Invalid Latitude. Value must be numeric.'));
          break;

        case $values['latitude'] > 90:
        case $values['latitude'] < -90:
          form_error($elements['latitude'], t('Invalid Latitude. Value must be between 90 and -90.'));
          break;
      }
    }

    if ($values['longitude'] !== "") {
      switch (TRUE) {
        case !is_numeric($values['longitude']):
          form_error($elements['longitude'], t('Invalid Longitude. Value must be numeric.'));
          break;

        case $values['longitude'] > 180:
        case $values['longitude'] < -180:
          form_error($elements['longitude'], t('Invalid Longitude. Value must be between 180 and -180.'));
          break;
      }
    }

    if (!is_numeric($values['search_distance']) || $values['search_distance'] < 0) {
      form_error($elements['search_distance'], t('Invalid Distance. Value must be a positive number.'));
    }

  }

  /**
   * Add this filter to the query.
   */
  function query() {
    if (empty($this->value)) {
      return;
    }

    $value = isset($this->value[0]) ? $this->value[0] : $this->value;

    if ($value['latitude'] === "" || $value['longitude'] === "") {
      return;
    }

    $this->ensure_my_table();
    $table = $this->table_alias;
    $field_id = str_replace('_distance', '', $this->options['field']);

    // Prepare filter values.
    $filter_distance = $value['search_distance'];
    $filter_lat = $value['latitude'];
    $filter_lng = $value['longitude'];

    // Prepare field values.
    $field_latsin = "{$table}.{$field_id}_lat_sin";
    $field_latcos = "{$table}.{$field_id}_lat_cos";
    $field_lng= "{$table}.{$field_id}_lng_rad";

    // Build the query.
    $sql = _proximity_sql_fragment($filter_lat, $filter_lng, $field_latsin, $field_latcos, $field_lng);

    // We use having to be able to reuse the query on field handlers
    $this->query->add_field(NULL, $sql, $this->field_alias);
    $this->query->add_having($this->options['group'], $this->field_alias, $filter_distance, $this->operator);
  }
}
