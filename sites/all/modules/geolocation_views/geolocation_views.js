(function($) {
  Drupal.geolocationViews = Drupal.geolocationViews || {};

  Drupal.behaviors.geolocationViews = {
    attach: function (context, settings) {
      $('.geolocation-views-map', context).each(function() {
        var $this = $(this);
        var mapId = this.id;
        Drupal.geolocationViews[mapId] = {};

        // Creating map
        var mapCenter = $this.data('map-center').split(',');
        Drupal.geolocationViews[mapId].map = new google.maps.Map(this, {
          center: new google.maps.LatLng(parseFloat(mapCenter[0]), parseFloat(mapCenter[1])),
          zoom: parseInt($this.data('map-zoom'), 10),
          maxZoom: parseInt($this.data('map-max-zoom'), 10) || null,
          minZoom: parseInt($this.data('map-min-zoom'), 10) || null,
          mapTypeId: google.maps.MapTypeId[$this.data('map-type')],
          scrollwheel: $this.data('scroll-wheel'),
          disableDoubleClickZoom: $this.data('disable-double-click-zoom')
        });

        // Creating Info Window
        Drupal.geolocationViews[mapId].infoWindow = new google.maps.InfoWindow();

        // Creating LatLngBounds
        var mapAutoCenter = $this.data('auto-center');
        if (mapAutoCenter) {
          Drupal.geolocationViews[mapId].markersBounds = new google.maps.LatLngBounds();
        }

        // Creating markers
        Drupal.geolocationViews[mapId].markers = [];
        $.each(Drupal.settings.geolocationViewsMarkers[mapId], function() {
          var markerPosition = new google.maps.LatLng(this.lat, this.lng);
          var marker = new google.maps.Marker({
            position: markerPosition,
            map: Drupal.geolocationViews[mapId].map,
            icon: this.icon,
            title: this.title,
            content: this.content,
            url: this.url,
            clickable: (this.title != '' || this.content != '' || this.url != '')
          });

          if (marker.url) {
            google.maps.event.addListener(marker, 'click', function() {
              location = marker.url;
            });
          }

          if (marker.content) {
            google.maps.event.addListener(marker, 'click', function() {
              Drupal.geolocationViews[mapId].infoWindow.setContent(marker.content);
              Drupal.geolocationViews[mapId].infoWindow.open(Drupal.geolocationViews[mapId].map, marker);
            });
          }

          if (mapAutoCenter) {
            Drupal.geolocationViews[mapId].markersBounds.extend(markerPosition);
          }
          
          Drupal.geolocationViews[mapId].markers.push(marker);
        });

        // Markers Clusterer
        if ($this.data('use-marker-clusterer')) {
          var markerClastererOptions = {gridSize: parseInt($this.data('marker-clusterer-grid-size'))};
          if ($this.data('marker-clusterer-max-zoom')) {
            markerClastererOptions.maxZoom = parseInt($this.data('marker-clusterer-max-zoom'));
          }
          if ($this.data('marker-clusterer-icon-url')) {
            var markerClastererSizes = $this.data('marker-clusterer-icon-size').split('x');
            markerClastererOptions.styles = [{
              url: $this.data('marker-clusterer-icon-url'),
              width: markerClastererSizes[0],
              height: markerClastererSizes[1]
            }];
          }
          Drupal.geolocationViews[mapId].markerClusterer = new MarkerClusterer(
            Drupal.geolocationViews[mapId].map,
            Drupal.geolocationViews[mapId].markers,
            markerClastererOptions
          );
        }

        if (mapAutoCenter) {
          Drupal.geolocationViews[mapId].map.setCenter(
            Drupal.geolocationViews[mapId].markersBounds.getCenter(),
            Drupal.geolocationViews[mapId].map.fitBounds(Drupal.geolocationViews[mapId].markersBounds)
          );
        }
      });
    }
  };
})(jQuery);