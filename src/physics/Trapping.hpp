/**
 * @file
 * @brief Definition of charge carrier trapping models
 *
 * @copyright Copyright (c) 2021 CERN and the Allpix Squared authors.
 * This software is distributed under the terms of the MIT License, copied verbatim in the file "LICENSE.md".
 * In applying this license, CERN does not waive the privileges and immunities granted to it by virtue of its status as an
 * Intergovernmental Organization or submit itself to any jurisdiction.
 */

#ifndef ALLPIX_TRAPPING_MODELS_H
#define ALLPIX_TRAPPING_MODELS_H

#include <TFormula.h>

#include "exceptions.h"

#include "core/utils/unit.h"
#include "objects/SensorCharge.hpp"

namespace allpix {

    /**
     * @ingroup Models
     * @brief Charge carrier trapping models
     */
    class TrappingModel {
    public:
        /**
         * Default constructor
         */
        TrappingModel() = default;

        /**
         * Default virtual destructor
         */
        virtual ~TrappingModel() = default;

        /**
         * Function call operator to obtain trapping time for the given carrier
         * @param type Type of charge carrier (electron or hole)
         * @param probability Current trapping probability for this charge carrier
         * @param timestep Current time step performed for the charge carrier
         * @param efield_mag Magnitude of the electric field
         * @return Trapping status and expected time of the charge carrier being trapped
         */
        virtual std::pair<bool, double>
        operator()(const CarrierType& type, double probability, double timestep, double efield_mag) const = 0;
    };

    /**
     * @ingroup Models
     * @brief No trapping
     *
     */
    class NoTrapping : virtual public TrappingModel {
    public:
        std::pair<bool, double> operator()(const CarrierType&, double, double, double) const override {
            return {false, 0.};
        };
    };

    /**
     * @ingroup Models
     * @brief Ljubljana / Kramberger effective trapping model for charge carriers in silicon
     *
     * Parametrization taken from https://doi.org/10.1016/S0168-9002(01)01263-3, effective trapping time from Eq. 4 with beta
     * values from Table 2 (pions/protons), temperature dependency according to Eq. 9, scaling factors kappa from Table 3.
     */
    class Ljubljana : virtual public TrappingModel {
    public:
        Ljubljana(double temperature, double fluence)
            : tau_eff_electron_(1. / Units::get(5.6e-16 * std::pow(temperature / 300, -0.86), "cm*cm/ns") / fluence),
              tau_eff_hole_(1. / Units::get(7.7e-16 * std::pow(temperature / 300, -1.52), "cm*cm/ns") / fluence) {}

        std::pair<bool, double>
        operator()(const CarrierType& type, double probability, double timestep, double) const override {
            return {probability <
                        (1 - std::exp(-1. * timestep / (type == CarrierType::ELECTRON ? tau_eff_electron_ : tau_eff_hole_))),
                    std::numeric_limits<double>::max()};
        };

    private:
        double tau_eff_electron_;
        double tau_eff_hole_;
    };

    /**
     * @ingroup Models
     * @brief Dortmund / Krasel effective trapping model for charge carriers in silicon
     *
     * Parametrization taken from https://doi.org/10.1109/TNS.2004.839096, effective trapping time from Eq. 3 with gamma
     * values from Eqs. 5 & 6
     */
    class Dortmund : virtual public TrappingModel {
    public:
        Dortmund()
            : tau_eff_electron_(1. / Units::get(5.13e-16, "cm*cm/ns")),
              tau_eff_hole_(1. / Units::get(5.04e-16, "cm*cm/ns")) {}

        std::pair<bool, double>
        operator()(const CarrierType& type, double probability, double timestep, double) const override {
            return {probability <
                        (1 - std::exp(-1. * timestep / (type == CarrierType::ELECTRON ? tau_eff_electron_ : tau_eff_hole_))),
                    std::numeric_limits<double>::max()};
        };

    private:
        double tau_eff_electron_;
        double tau_eff_hole_;
    };

    /**
     * @ingroup Models
     * @brief Effective trapping model developed by the CMS Tracker Group
     *
     * Parametrization taken from https://doi.org/10.1088/1748-0221/11/04/P04023, effective trapping time from Table 2.
     * Interpolation between evaluated fluence points by M. Bomben
     *
     * FIXME no temperature dependence
     */
    class CMSTracker : virtual public TrappingModel {
    public:
        CMSTracker(double fluence)
            : tau_eff_electron_(1. / (Units::get(1.71e-16, "cm*cm/ns") * fluence + Units::get(0.114, "/ns"))),
              tau_eff_hole_(1. / (Units::get(2.79e-16, "cm*cm/ns") * fluence + Units::get(0.093, "/ns"))) {}

        std::pair<bool, double>
        operator()(const CarrierType& type, double probability, double timestep, double) const override {
            return {probability <
                        (1 - std::exp(-1. * timestep / (type == CarrierType::ELECTRON ? tau_eff_electron_ : tau_eff_hole_))),
                    std::numeric_limits<double>::max()};
        };

    private:
        double tau_eff_electron_;
        double tau_eff_hole_;
    };

    /**
     * @ingroup Models
     * @brief Custom trapping model for charge carriers
     */
    class CustomTrapping : virtual public TrappingModel {
    public:
        CustomTrapping(const Configuration& config) {
            tau_eff_electron_ = configure_tau_eff(config, CarrierType::ELECTRON);
            tau_eff_hole_ = configure_tau_eff(config, CarrierType::HOLE);
        };

        std::pair<bool, double>
        operator()(const CarrierType& type, double probability, double timestep, double efield_mag) const override {
            return {probability <
                        (1 - std::exp(-1. * timestep /
                                      (type == CarrierType::ELECTRON ? tau_eff_electron_->Eval(timestep, efield_mag)
                                                                     : tau_eff_hole_->Eval(timestep, efield_mag)))),
                    std::numeric_limits<double>::max()};
        };

    private:
        std::unique_ptr<TFormula> tau_eff_electron_;
        std::unique_ptr<TFormula> tau_eff_hole_;

        std::unique_ptr<TFormula> configure_tau_eff(const Configuration& config, const CarrierType type) {
            std::string name = (type == CarrierType::ELECTRON ? "electrons" : "holes");
            auto function = config.get<std::string>("trapping_function_" + name);
            auto parameters = config.getArray<double>("trapping_parameters_" + name, {});

            auto trapping = std::make_unique<TFormula>(("trapping_" + name).c_str(), function.c_str());

            if(!trapping->IsValid()) {
                throw InvalidValueError(
                    config, "trapping_function_" + name, "The provided model is not a valid ROOT::TFormula expression");
            }

            // Check if number of parameters match up
            if(static_cast<size_t>(trapping->GetNpar()) != parameters.size()) {
                throw InvalidValueError(config,
                                        "trapping_parameters_" + name,
                                        "The number of provided parameters and parameters in the function do not match");
            }

            // Set the parameters
            for(size_t n = 0; n < parameters.size(); ++n) {
                trapping->SetParameter(static_cast<int>(n), parameters[n]);
            }

            return trapping;
        };
    };

    /**
     * @brief Wrapper class and factory for trapping models.
     *
     * This class allows to store trapping  objects independently of the model chosen and simplifies access to the
     * function call operator. The constructor acts as factory, generating model objects from the model name provided, e.g.
     * from a configuration file.
     */
    class Trapping {
    public:
        /**
         * Default constructor
         */
        Trapping() = default;

        /**
         * Recombination constructor
         * @param config Configuration of the calling module
         */
        Trapping(const Configuration& config) {
            try {
                auto model = config.get<std::string>("trapping_model", "none");
                std::transform(model.begin(), model.end(), model.begin(), ::tolower);
                auto temperature = config.get<double>("temperature");
                auto fluence = config.get<double>("fluence", 0);

                if(model == "ljubljana" || model == "kramberger") {
                    model_ = std::make_unique<Ljubljana>(temperature, fluence);
                } else if(model == "dortmund" || model == "krasel") {
                    model_ = std::make_unique<Dortmund>();
                } else if(model == "none") {
                    LOG(INFO) << "No charge carrier trapping model chosen, no trapping simulated";
                    model_ = std::make_unique<NoTrapping>();
                } else if(model == "custom") {
                    model_ = std::make_unique<CustomTrapping>(config);
                } else {
                    throw InvalidModelError(model);
                }
                LOG(DEBUG) << "Selected trapping model \"" << model << "\"";
            } catch(const ModelError& e) {
                throw InvalidValueError(config, "trapping_model", e.what());
            }
        }

        /**
         * Function call operator forwarded to the trapping model
         * @return Trapping time
         */
        template <class... ARGS> std::pair<bool, double> operator()(ARGS&&... args) const {
            return model_->operator()(std::forward<ARGS>(args)...);
        }

    private:
        std::unique_ptr<TrappingModel> model_{};
    };

} // namespace allpix

#endif
