/*
 * \brief  Sculpt framebuffer management
 * \author Roman Iten
 * \date   2018-08-18
 */

/* local includes */
#include <framebuffer.h>
#include <model/fb_connectors.h>


void Sculpt::Framebuffer::handle_fb_connectors_update()
{
	_fb_connectors_rom.update();

	Fb_connector_update_policy policy(_alloc, _fb_connectors);
	_fb_connectors.update_from_xml(policy, _fb_connectors_rom.xml());

	unsigned int connector_connected_count = 0;
	unsigned int connector_enabled_count = 0;

	_fb_connectors.for_each([&] (Fb_connector const& connector) {
		if (connector.connected) connector_connected_count++;
		if (connector.enabled) connector_enabled_count++;
	});

	unsigned max_width = 0;
	unsigned max_height = 0;
	bool stoploop;

	if (connector_connected_count == 1) {
		_fb_connectors.for_each([&] (Fb_connector &connector) {
			if (connector.connected) {
				connector.modes.for_each([&] (Fb_connector_mode const& mode) {
					if (mode.width*mode.height > max_width*max_height) {
						max_width = mode.width;
						max_height = mode.height;
					}
				});
			}
		});

		_fb_connectors.for_each([&] (Fb_connector &connector) {
			if (connector.connected) {
				stoploop = false;
				connector.modes.for_each([&] (Fb_connector_mode const& mode) {
					if (!stoploop && mode.width == max_width && mode.height == max_height) {
						connector.set_enabled(true);
						connector.select_mode(mode.id());
						stoploop = true;
					}
				});
			}
		});
	}

	else {
		_fb_connectors.for_each([&] (Fb_connector const& connector1) {
			if (connector1.connected) {
				connector1.modes.for_each([&] (Fb_connector_mode const& mode1) {
					_fb_connectors.for_each([&] (Fb_connector const& connector2) {
						if (connector2.connected && connector1.name != connector2.name) {
							connector2.modes.for_each([&] (Fb_connector_mode const& mode2) {
								if (mode1.width == mode2.width && mode1.height == mode2.height && (mode1.width*mode1.height > max_width*max_height)) {
									max_width = mode1.width;
									max_height = mode1.height;
								}
							});
						}
					});
				});
			}
		});

		if ((max_width == 0) || (max_height == 0)) {
			warning("no common resolution found, using fallback (", _fallback_width, "x", _fallback_height, ")");
			max_width = _fallback_width;
			max_height = _fallback_height;
		}

		_fb_connectors.for_each([&] (Fb_connector &connector) {
			stoploop = false;
			connector.modes.for_each([&] (Fb_connector_mode const& mode) {
				if (!stoploop && mode.height == max_height && mode.width == max_width) {
					connector.set_enabled(true);
					connector.select_mode(mode.id());
					stoploop = true;
				}
			});
		});
	}




	_generate_fb_drv_config();

	_dialog_generator.generate_dialog();
}


void Sculpt::Framebuffer::_generate_fb_drv_config()
{
	unsigned int connected_connector_count = 0;

	_fb_connectors.for_each([&] (Fb_connector const/*& connector*/) {
		connected_connector_count++;
	});

	// Booting with multiple monitors attached:
	/*modes_enabled_count = 0;
	_fb_connectors.for_each([&] (Fb_connector const& connector) {
		if (connector.connected) {
			connector.modes.for_each([&] (Fb_connector_mode const& mode) {
				if (mode.enabled) modes_enabled_count++;

			});
		}
	});

	if (modes_enabled_count == 0) {
		// Yes, we just booted!
	}


	if (connector_count == 1) {
		_fb_connectors.for_each([&] (Fb_connector const& connector) {
			if (connector.connected) {
				modes_enabled_count = 0;
				connector.modes.for_each([&] (Fb_connector_mode const& mode) {
					if (mode.enabled) modes_enabled_count++;
				});

				if (modes_enabled_count == 0) {
					connector.modes.for_each([&] (Fb_connector_mode const& mode) {
						if (mode.common(_fb_connectors) && ((mode.width*mode.height) > (max_width*max_height))) {
							max_width = mode.width;
							max_height = mode.height;
						}
					});
				}

				
			}
		});

		if ((max_width == 0) || (max_height == 0)) {
			warning("no common resolution found, using fallback (", _fallback_width, "x", _fallback_height, ")");
			max_width = _fallback_width;
			max_height = _fallback_height;
		}
	}*/

	_fb_drv_config.generate([&] (Xml_generator &xml) {
		xml.attribute("buffered", "yes");

		xml.node("report", [&] () {
			xml.attribute("connectors", "yes");
		});

		_fb_connectors.for_each([&] (Fb_connector const& connector) {
			if (connector.connected/* && connector.enabled */) {
				xml.node("connector", [&] () {
					xml.attribute("name", connector.name);
					xml.attribute("width", connector.selected_mode().width);
					xml.attribute("height", connector.selected_mode().height);
					xml.attribute("hz", connector.selected_mode().hz);
					xml.attribute("enabled", connector.enabled);
				});
			}
		});
	});
}
