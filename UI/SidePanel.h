#ifndef _SidePanel_h_
#define _SidePanel_h_

#include "CUIControls.h"
#include "CUIWnd.h"

#include "../universe/ResourceCenter.h"
#include "../universe/Planet.h"
#include "../universe/System.h"

#include <GG/DynamicGraphic.h>
#include <GG/SignalsAndSlots.h>
#include <GG/Texture.h>

#include <vector>

class CUIDropDownList;
class MultiIconValueIndicator;

class SidePanel : public CUIWnd {
public:
    class PlanetPanel;

    /** \name Structors */ //@{
    SidePanel(const std::string& config_name);
    ~SidePanel();
    //@}

    /** \name Accessors */ //@{
    virtual bool        InWindow(const GG::Pt& pt) const;
    virtual GG::Pt      ClientUpperLeft() const;

    /** Returns the id of the system shown in the SidePanels, or
      * INVALID_OBJECT_ID if no system is shown */
    static int          SystemID();

    /** Returns the id of the currently-selected planet, or
      * INVALID_OBJECT_ID if no planet is selected */
    int                 SelectedPlanetID() const;

    /** Returns whether this SidePanel contains an object with the indicated
      * \a object_id that can be selected within the SidePanel. */
    bool                PlanetSelectable(int id) const;
    //@}

    /** \name Mutators */ //@{
    virtual void        PreRender();
    virtual void        Render();
    virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr);

    /** Updates contents of sidepanel, but doesn't recreate contents or check
      * that object pointers are still valid.  All SidePanels' are updated */
    static void         Update();

    /** Make refresh of sidepanel and contents happen during
        pre-render. PreRender() Will recreate all contents from the stored
        system id. All SidePanels are refreshed. */
    static void         Refresh();

    /** Selects the planet with id \a planet_id within the current system, if
      * such a planet exists.  All SidePanels' selected planets are set, if
      * those panels have planet selection enabled. */
    static void         SelectPlanet(int planet_id);

    /** Sets the system currently being viewed in all side panels */
    static void         SetSystem(int system_id);

    /** Enables, or disables if \a enable is false, selection of planet panels
      * within this SidePanel.  Panels that can be selected are those which
      * meet selection conditions that need to be figured out and perhaps
      * made more configurable, but for now are just those planets owned by
      * this client's empire. */
    void                EnableSelection(bool enable = true);

    /** Enables, or disables if \a enable is false, issuing orders via the SidePanel. */
    void                EnableOrderIssuing(bool enable = true);
    //@}

    /** emitted when a rotating planet in the side panel is clicked by the
      * user */
    static boost::signals2::signal<void (int)>    PlanetSelectedSignal;

    /** emitted when something in the sidepanel wants to change the selected
      * system, including the droplist or back/forward arrows */
    static boost::signals2::signal<void (int)>    SystemSelectedSignal;

    /** emitted when a planet's resourcecenter has changed, including when
      * focus is changed */
    static boost::signals2::signal<void ()>       ResourceCenterChangedSignal;

    /** emitted when a planet is left double clicked*/
    static boost::signals2::signal<void (int)>    PlanetDoubleClickedSignal;

    /** emitted when a planet is right clicked */
    static boost::signals2::signal<void (int)>    PlanetRightClickedSignal;

    /** emitted when a building is right clicked */
    static boost::signals2::signal<void (int)>    BuildingRightClickedSignal;

protected:
    virtual void        InitBuffers();

private:
    class PlanetPanelContainer;

    void                DoLayout();

    void                UpdateImpl();                   ///< updates contents quickly.  to be used when meters or other objects' data changes

    /** Fully refreshes sidepanel and contents, recreating all contents from
      * stored system id.  All SidePanels are refreshed. */
    void                RefreshInPreRender();

    void                RefreshImpl();                  ///< fully refreshes contents.  to be used when objects are created, destroyed or added to system
    void                SelectPlanetImpl(int planet_id);///< sets selected planet in this sidepanel

    /**  Insert all known systems into the SystemName drop down list.*/
    void                RefreshSystemNames();
    /**  Refresh the system name list when it closes, in case the known systems changed while it
         was open. */
    void                SystemNameDropListOpenedSlot(bool is_open);
    /**  Handle the user selecting a system in the droplist while the list is closed, using keys.
         It may emit SystemSelectedSignal. */
    void                SystemSelectionChangedSlot(GG::DropDownList::iterator it);

    void                PrevButtonClicked();            ///< responds to user clicking next system button
    void                NextButtonClicked();            ///< responts to user clicking previous system button
    void                PlanetSelected(int planet_id);  ///< responds to user selection of a planet by emitting PlanetSelectedSignal

    static void         FleetsInserted(const std::vector<TemporaryPtr<Fleet> >& fleets);    ///< responds to insertion fleets into system during a turn.  may update colonize buttons
    static void         FleetsRemoved(const std::vector<TemporaryPtr<Fleet> >& fleets);     ///< responds to removal fleets from system during a turn.  may update colonize buttons
    static void         FleetStateChanged();            ///< responds to fleet state changes during a turn, which may include issueing or cancelling move orders.  may update colonize buttons

    class SystemNameDropDownList;
    SystemNameDropDownList*     m_system_name;
    GG::TextControl*            m_star_type_text;
    GG::Button*                 m_button_prev;
    GG::Button*                 m_button_next;
    GG::DynamicGraphic*         m_star_graphic;

    std::vector<GG::SubTexture> m_fleet_icons;

    PlanetPanelContainer*       m_planet_panel_container;
    MultiIconValueIndicator*    m_system_resource_summary;

    bool                        m_selection_enabled;

    static bool                 s_needs_update;
    static bool                 s_needs_refresh;

    static int                  s_system_id;

    static std::set<SidePanel*> s_side_panels;

    static std::set<boost::signals2::connection>      s_system_connections;
    static std::map<int, boost::signals2::connection> s_fleet_state_change_signals;
};

TemporaryPtr<const Ship>    ValidSelectedColonyShip(int system_id);
int                         AutomaticallyChosenColonyShip(int target_planet_id);

#endif // _SidePanel_h_
