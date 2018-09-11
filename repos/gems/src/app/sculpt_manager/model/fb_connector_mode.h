/*
 * \brief  Representation of a framebuffer connector
 * \author Roman Iten
 * \date   2018-08-18
 */

#ifndef _MODEL__FB_CONNECTOR_MODE_H_
#define _MODEL__FB_CONNECTOR_MODE_H_

#include "types.h"
#include "fb_connectors.h"

namespace Sculpt {
	struct Fb_connector_mode;
	struct Fb_connector_mode_update_policy;

	typedef List_model<Fb_connector_mode> Fb_connector_modes;
};


struct Sculpt::Fb_connector_mode : List_model<Fb_connector_mode>::Element
{
	unsigned width {0} ;
	unsigned height {0} ;
	unsigned hz {0} ;

	typedef String<16> Id;

	Fb_connector_mode(unsigned width, unsigned height, unsigned hz)
	: width(width), height(height), hz(hz) { }

	Fb_connector_mode() {}

	Id id() const { return Id(width, "x" , height, "@", hz); }

/*	bool common(Fb_connectors const& connectors) const
	{
		unsigned connector_count = 0;
		unsigned found_count = 0;

		connectors.for_each([&] (Fb_connector const& connector) {
			if (connector.connected)
				++connector_count;
			connector.modes.for_each([&] (Fb_connector_mode const& mode) {
				// We shouldn't be checking for Hz.
				// I'll leave it commented out while I get everything else to work.
				//if (mode.width == width && mode.height == height)
				if (mode.id() == id())
					++found_count;
			});
		});

		return (found_count >= connector_count);
	}
	*/
};

/**
 * Policy for transforming a 'connector' node into a list model
 */
struct Sculpt::Fb_connector_mode_update_policy : List_model<Fb_connector_mode>::Update_policy
{
	Allocator& _alloc;
	//Fb_connectors& _connectors;

	Fb_connector_mode_update_policy(Allocator &alloc/*, Fb_connectors& connectors*/)
	: _alloc(alloc)/*, _connectors(connectors)*/ { }

	void destroy_element(Fb_connector_mode &elem)
	{
		destroy(_alloc, &elem);
	}

	Fb_connector_mode &create_element(Xml_node const& node)
	{
		Fb_connector_mode& elem = *new (_alloc)
			Fb_connector_mode(node.attribute_value("width",  0u),
			                  node.attribute_value("height", 0u),
			                  node.attribute_value("hz",     0u));

		return elem;
	}

	//void update_element(Fb_connector_mode &elem, const Xml_node& node)
	void update_element(Fb_connector_mode /* &elem */, const Xml_node& /*node*/)
	{
		//elem.enabled = node.attribute_value("enabled", 0u);
	}

	static bool element_matches_xml_node(Fb_connector_mode const& elem, Xml_node const& node)
	{
		return (node.attribute_value("width",  0u) == elem.width)
		    && (node.attribute_value("height", 0u) == elem.height)
		    && (node.attribute_value("hz",     0u) == elem.hz);
	}
};

#endif /* _MODEL__FB_CONNECTOR_MODE_H_ */
