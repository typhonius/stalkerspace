
(function ($) {

  Drupal.behaviors.addGMapCurrentLocation = {
    attach: function (context, settings) {

      if (typeof(google) != 'object') { // when not connected to Internet
        return;
      }
      // Start with a map canvas, then add marker and balloon with address info
      // when the geo-position comes in.
      var mapOptions = settings.ip_geoloc_current_location_map_options;
      if (!mapOptions) {
        mapOptions = { mapTypeId: google.maps.MapTypeId.ROADMAP, zoom: 15 };
      }
      var map = new google.maps.Map(document.getElementById(settings.ip_geoloc_current_location_map_div), mapOptions);

      /* Use the geo.js unified API. This covers the W3C Geolocation API
       * as well as some specific devices like Palm and Blackberry.
       */
      if (typeof(geo_position_js) == 'object' && geo_position_js.init()) {
        // Note that we use the same function for normal and error behaviours.
        geo_position_js.getCurrentPosition(displayMap, displayMap, {enableHighAccuracy: true});
      }
      else {
        // Don't pop up annoying alert. Just show blank map of the world.
        map.setZoom(0);
        map.setCenter(new google.maps.LatLng(0, 0));
      }

      function displayMap(position) {
        var coords = null;
        if (position.coords) {
          coords = position.coords;
        }
        else {
          // If the user declinesd to share their location or if there was some
          // other error, we set the center based on the fixed lat/long passed
          // in via the settings. Without a center there won't be a map!
          // If not supplied this results in a marker in the ocean at (0,0).
          var latLng = settings.ip_geoloc_current_location_map_latlng;
          coords = new Object;
          coords.latitude = latLng[0];
          coords.longitude = latLng[1];
        }
        var center = new google.maps.LatLng(coords.latitude, coords.longitude);
        map.setCenter(center);
        var marker = new google.maps.Marker({ map: map, position: center });
        new google.maps.Geocoder().geocode({'latLng': center}, function(response, status) {
          if (status == google.maps.GeocoderStatus.OK) {
            addressText = response[0]['formatted_address'];
          }
          else {
          //alert(Drupal.t('IPGV&M: Google address lookup failed with status code !code.', { '!code': status }));
          }
          // lat/long and address are revealed when clicking marker
          var lat = coords.latitude.toFixed(4);
          var lng = coords.longitude.toFixed(4);
          var latLongText = Drupal.t('lat. !lat, long. !long', { '!lat': lat, '!long': lng }) + '<br/>'
            + Drupal.t('accuracy !accuracy m', { '!accuracy': coords.accuracy });
          var infoPopUp = new google.maps.InfoWindow({ content: addressText + '<br/>' + latLongText });
          google.maps.event.addListener(marker, 'click', function() { infoPopUp.open(map, marker) });
          //google.maps.event.addListener(map, 'center_changed', function() {
          //  alert('New coords: ' + map.getCenter().lat() + ', ' + map.getCenter().lng());
          //});
        });
      }
    }
  }
})(jQuery);
