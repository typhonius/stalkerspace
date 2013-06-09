(function ($) {

  Drupal.behaviors.addGMapMultiLocation = {
    attach: function (context, settings) {

      var locations     = settings.ip_geoloc_locations;
      var mapOptions    = settings.ip_geoloc_multi_location_map_options;
      var centerOption  = settings.ip_geoloc_multi_location_center_option;
      var visitorMarker = settings.ip_geoloc_multi_location_visitor_marker;
      var use_gps       = settings.ip_geoloc_multi_location_visitor_location_gps;
      var markerDirname = settings.ip_geoloc_multi_location_marker_directory;
      var markerWidth   = settings.ip_geoloc_multi_location_marker_width;
      var markerHeight  = settings.ip_geoloc_multi_location_marker_height;
      var markerAnchor  = settings.ip_geoloc_multi_location_marker_anchor;
      var markerColor   = settings.ip_geoloc_multi_location_marker_default_color;
      var imageExt      = '.png';

      if (!mapOptions) {
        alert(Drupal.t('Syntax error in map options.'));
      }
      if (typeof(google) != 'object') { // when not connected to Internet
        return;
      }
      var mapDiv = document.getElementById(settings.ip_geoloc_multi_location_map_div);
      // Create map as a global, see [#1954876]
      map = new google.maps.Map(mapDiv, mapOptions);

      // A map must have a type, a zoom and a center or nothing will show.
      if (!map.getMapTypeId()) {
        map.setMapTypeId(google.maps.MapTypeId.ROADMAP);
      }
      if (!map.getZoom()) {
        map.setZoom(1);
      }

      if (centerOption != 1 || locations.length == 0) {
        // If no center override option was specified or as a fallback where
        // the user declines to share their location, we set the center based
        // on the mapOptions. Without a center there won't be a map!
        map.setCenter(new google.maps.LatLng(
          mapOptions.centerLat ? mapOptions.centerLat : 0,
          mapOptions.centerLng ? mapOptions.centerLng : 0));
      }

      if (visitorMarker || centerOption == 2) {
        // Retrieve visitor's location, fall back on supplied location, if not found.
        if (use_gps && typeof(geo_position_js) == 'object' && geo_position_js.init()) {
          // Center the map on the user's current location, using the unified API.
          geo_position_js.getCurrentPosition(handleMapCenterAndVisitorMarker1, handlePositionError, {enableHighAccuracy: true});
        }
        else {
          // Use supplied visitor lat/lng to center and set marker.
          var latLng = settings.ip_geoloc_multi_location_center_latlng;
          if (latLng) {
            handleMapCenterAndVisitorMarker2(latLng[0], latLng[1]);
          }
        }
      }

      var defaultPinImage = !markerColor ? null : new google.maps.MarkerImage(
        markerDirname + '/' + markerColor + imageExt,
        new google.maps.Size(markerWidth, markerHeight),
        new google.maps.Point(0, 0), // origin
        new google.maps.Point((markerWidth / 2), markerAnchor)); // anchor
      var shadowImage = null;

      var i = 1;
      var balloonTexts = [];
      var center = null;
      for (var key in locations) {
        var mouseOverText = Drupal.t('Location #@i', { '@i': i++ });
        var position = new google.maps.LatLng(locations[key].latitude, locations[key].longitude);
        if (!center && centerOption == 1) {
          // If requested center map on the first location, if any.
          map.setCenter(position);
          center = position;
        }
        var pinImage = defaultPinImage;
        if (locations[key].marker_color) {
          pinImage = new google.maps.MarkerImage(
            markerDirname + '/' + locations[key].marker_color + imageExt,
            new google.maps.Size(markerWidth, markerHeight),
            new google.maps.Point(0, 0), // origin
            new google.maps.Point((markerWidth / 2), markerAnchor)); // anchor
        }
        marker = new google.maps.Marker({ map: map, icon: pinImage, shadow: shadowImage, position: position, title: mouseOverText });

        // Funny index is because listener callback only gives us position
        balloonTexts['LL' + position] = '<div class="balloon">'  + locations[key].balloon_text + '</div>';
        google.maps.event.addListener(marker, 'click', function(event) {
          new google.maps.InfoWindow({
            content: balloonTexts['LL' + event.latLng],
            position: event.latLng,
            maxWidth: 200 // [#1777664]
          }).open(map);
        });
      }

      function handleMapCenterAndVisitorMarker1(visitorPosition) {
        handleMapCenterAndVisitorMarker2(visitorPosition.coords.latitude, visitorPosition.coords.longitude);
      }

      function handleMapCenterAndVisitorMarker2(latitude, longitude) {
        var visitorPosition = new google.maps.LatLng(latitude, longitude);
        if (centerOption == 2) {
          map.setCenter(visitorPosition);
        }
        if (visitorMarker) {
          showSpecialMarker(visitorPosition, Drupal.t('Your retrieved position (' + latitude + ', ' + longitude + ')'));
        }
      }

      function showSpecialMarker(position, text) {
        if (visitorMarker == true) {
          specialMarker = new google.maps.Marker({ map: map, position: position, title: text });
        }
        else {
          // Interpret value of visitorMarker as the marker color
          var pinColor = visitorMarker; // eg "00FF00" for bright green
          var pinChar = "%E2%80%A2"; // use character like "x", for a dot use "%E2%80%A2"
          var textColor = "000000";  // black
          // Note: cannot use https: here...
          var pinImage = new google.maps.MarkerImage("http://chart.apis.google.com/chart?chst=d_map_pin_letter&chld=" + pinChar + "|" + pinColor + "|" + textColor,
            new google.maps.Size(21, 34), new google.maps.Point(0, 0), new google.maps.Point(10, 34));
          specialMarker = new google.maps.Marker({ map: map, icon: pinImage, shadow: shadowImage, position: position, title: text });
        }
        google.maps.event.addListener(specialMarker, 'click',  function(event) {
          new google.maps.InfoWindow({
            content: text,
            position: event.latLng
          }).open(map);
        });
      }

      // Fall back on IP address lookup, for instance when user declined to share location (error 1)
      function handlePositionError(error) {
        //alert(Drupal.t('IPGV&M multi-location map: getCurrentPosition() returned error !code', {'!code': error.code}));
        var latLng = settings.ip_geoloc_multi_location_center_latlng;
        if (latLng) {
          handleMapCenterAndVisitorMarker2(latLng[0], latLng[1]);
        }
      }
    }
  }
})(jQuery);
