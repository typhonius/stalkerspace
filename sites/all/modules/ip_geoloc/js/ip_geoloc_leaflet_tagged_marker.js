(function ($) {

  Drupal.leaflet._create_point_orig = Drupal.leaflet.create_point;

  Drupal.leaflet.create_point = function(marker) {

    // Follow create_point()
    var latLng = new L.LatLng(marker.lat, marker.lon);
    this.bounds.push(latLng);

    if (!marker.tag) {
      if (marker.icon == false) {
        // Need to create a marker "stub" or we'll have no map at all!
        var stub = new L.Icon({iconUrl: '//'});
        return new L.Marker(latLng, {icon: stub, title: marker.tooltip});
      }
      if (!marker.icon) {
        // Default icon without tag
        return new L.Marker(latLng, {title: marker.tooltip});
      }
    }
    // Tagged marker (with or without icon) or untagged marker with specified icon.
    if (marker.icon == false) {
      var divIcon = new L.DivIcon({html: marker.tag, className: marker.cssClass});
      // Prevent div style tag being set, so that upper left corner becomes anchor.
      divIcon.options.iconSize = null;
      return new L.Marker(latLng, {icon: divIcon, title: marker.tooltip});
    }
    
    if (marker.tag && !marker.icon) { // use default icon and tag it
      var tagged_icon = new L.Icon.Tagged(marker.tag, {className: marker.cssClass});
      return new L.Marker(latLng, {icon: tagged_icon, title: marker.tooltip});
    }
    var icon = marker.tag
      ? new L.Icon.Tagged(marker.tag, {iconUrl: marker.icon.iconUrl, className: marker.cssClass})
      : new L.Icon({iconUrl: marker.icon.iconUrl});
    // All of the below is like create_point (leaflet.drupal.js), but with tooltip.
    if (marker.icon.iconSize) {
      icon.options.iconSize = new L.Point(parseInt(marker.icon.iconSize.x), parseInt(marker.icon.iconSize.y));
    }
    if (marker.icon.iconAnchor) {
      icon.options.iconAnchor = new L.Point(parseFloat(marker.icon.iconAnchor.x), parseFloat(marker.icon.iconAnchor.y));
    }
    if (marker.icon.popupAnchor) {
      icon.options.popupAnchor = new L.Point(parseFloat(marker.icon.popupAnchor.x), parseFloat(marker.icon.popupAnchor.y));
    }
    if (marker.icon.shadowUrl !== undefined) {
      icon.options.shadowUrl = marker.icon.shadowUrl;
    }
    if (marker.icon.shadowSize) {
      icon.options.shadowSize = new L.Point(parseInt(marker.icon.shadowSize.x), parseInt(marker.icon.shadowSize.y));
    }
    if (marker.icon.shadowAnchor) {
      icon.options.shadowAnchor = new L.Point(parseInt(marker.icon.shadowAnchor.x), parseInt(marker.icon.shadowAnchor.y));
    }
    return new L.Marker(latLng, {icon: icon, title: marker.tooltip});
  };

})(jQuery);

L.Icon.Tagged = L.Icon.extend({

  initialize: function (tag, options) {
    L.Icon.prototype.initialize.apply(this, [options]);
    this._tag = tag;
  },
 
  // Create an icon as per normal, but wrap it in an outerdiv together with the tag.
  createIcon: function() {
    if (!this.options.iconUrl) {
      var iconDefault = new L.Icon.Default();
      this.options.iconUrl = iconDefault._getIconUrl('icon');
      this.options.iconSize = iconDefault.options.iconSize;
		  this.options.iconAnchor = iconDefault.options.iconAnchor;
		  this.options.popupAnchor = iconDefault.options.popupAnchor; // does this work?
      this.options.shadowSize = iconDefault.options.shadowSize;
    }
    var img = this._createIcon('icon');
    var tag = document.createElement('div');
    tag.innerHTML = this._tag;
    if (this.options.className) {
      tag.setAttribute('class', this.options.className);
    }
    var outer = document.createElement('div');
    outer.setAttribute('class', 'leaflet-tagged-marker');
    // The order of these makes little difference
    outer.appendChild(img);
    outer.appendChild(tag);
    return outer;
  },

  createShadow: function() { 
    return this._createIcon('shadow');
  }
});
