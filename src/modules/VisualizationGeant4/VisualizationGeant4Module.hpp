/*
 * Visualization module
 */

#ifndef ALLPIX_TEST_VISUALIZATION_MODULE_H
#define ALLPIX_TEST_VISUALIZATION_MODULE_H

#include <memory>
#include <string>

#include "core/config/Configuration.hpp"
#include "core/geometry/GeometryManager.hpp"
#include "core/messenger/Messenger.hpp"
#include "core/module/Module.hpp"

class G4UIExecutive;
class G4VisManager;

namespace allpix {
    // define the module to inherit from the module base class
    class VisualizationGeant4Module : public Module {
    public:
        // constructor should take a pointer to the Configuration, the Messenger and the Geometry Manager
        VisualizationGeant4Module(Configuration config, Messenger*, GeometryManager*);
        ~VisualizationGeant4Module() override;

        // initializes the visualization and set necessary settings to catch all the required data
        void init() override;

        // sets the display settings for the current event
        void run(unsigned int) override;

        // displays the visualization
        void finalize() override;

    private:
        // configuration for this module
        Configuration config_;

        // variable to check if we did run succesfullly
        bool has_run_;

        // pointer to the visualization manager
        std::unique_ptr<G4VisManager> vis_manager_g4_;
        std::unique_ptr<G4UIExecutive> ui_window_;
    };
} // namespace allpix

#endif /* ALLPIX_TEST_VISUALIZATION_MODULE_H */
