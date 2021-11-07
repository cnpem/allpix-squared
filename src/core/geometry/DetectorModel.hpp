/**
 * @file
 * @brief Base of detector models
 *
 * @copyright Copyright (c) 2017-2022 CERN and the Allpix Squared authors.
 * This software is distributed under the terms of the MIT License, copied verbatim in the file "LICENSE.md".
 * In applying this license, CERN does not waive the privileges and immunities granted to it by virtue of its status as an
 * Intergovernmental Organization or submit itself to any jurisdiction.
 * SPDX-License-Identifier: MIT
 */

/**
 * @defgroup DetectorModels Detector models
 * @brief Collection of detector models supported by the framework
 */

#ifndef ALLPIX_DETECTOR_MODEL_H
#define ALLPIX_DETECTOR_MODEL_H

#include <array>
#include <string>
#include <utility>

#include <Math/Point2D.h>
#include <Math/Point3D.h>
#include <Math/Vector2D.h>
#include <Math/Vector3D.h>

#include "core/config/ConfigReader.hpp"
#include "core/config/exceptions.h"
#include "core/utils/log.h"
#include "objects/Pixel.hpp"
#include "tools/ROOT.h"

#include "DetectorAssembly.hpp"
#include "SupportLayer.hpp"

namespace allpix {
    /**
     * @brief Sensor materials
     */
    enum class SensorMaterial {
        SILICON = 1,            ///< Silicon
        GALLIUM_ARSENIDE,       ///< Gallium Arsenide
        GERMANIUM,              ///< Germanium
        CADMIUM_TELLURIDE,      ///< Cadmium Telluride
        CADMIUM_ZINC_TELLURIDE, ///< Cadmium Zinc Telluride
        DIAMOND,                ///< Diamond
        SILICON_CARBIDE,        ///< Silicon Carbide
        GALLIUM_NITRIDE,        ///< Gallium Nitride
    };

    /**
     * @ingroup DetectorModels
     * @brief Base of all detector models
     *
     * Implements the minimum required for a detector model. A model always has a pixel grid with a specific pixel size. The
     * pixel grid defines the base size of the sensor, chip and support. Excess length can be specified. Every part of the
     * detector model has a defined center and size which can be overloaded by specialized detector models. The basic
     * detector model also defines the rotation center in the local coordinate system.
     */
    class DetectorModel {
    public:
        /**
         * @brief Factory to dynamically create detector model objects
         * @param name Name of the model
         * @param reader Reader with the configuration for this model
         * @return Detector model instantiated from the configuration
         */
        static std::shared_ptr<DetectorModel> factory(const std::string& name, const ConfigReader&);

        /**
         * @brief Helper method to determine if this detector model is of a given type
         * The template parameter needs to be specified specifically, i.e.
         *     if(model->is<PixelDetectorModel>()) { }
         * @return Boolean indication whether this model is of the given type or not
         */
        template <class T> bool is() { return dynamic_cast<T*>(this) != nullptr; }

        /**
         * @brief Constructs the base detector model
         * @param type Name of the model type
         * @param assembly Detector assembly object with information about ASIC and packaging
         * @param reader Configuration reader with description of the model
         */
        explicit DetectorModel(std::string type, std::shared_ptr<DetectorAssembly> assembly, ConfigReader reader);

        /**
         * @brief Essential virtual destructor
         */
        virtual ~DetectorModel() = default;

        ///@{
        /**
         * @brief Use default copy and move behaviour
         */
        DetectorModel(const DetectorModel&) = default;
        DetectorModel& operator=(const DetectorModel&) = default;

        DetectorModel(DetectorModel&&) = default;
        DetectorModel& operator=(DetectorModel&&) = default;
        ///@}

        /**
         * @brief Get the configuration associated with this model
         * @return Configuration used to construct the model
         */
        std::vector<Configuration> getConfigurations() const;

        /**
         * @brief Get the type of the model
         * @return Model type
         */
        std::string getType() const { return type_; }

        const std::shared_ptr<DetectorAssembly> getAssembly() const { return assembly_; }

        /**
         * @brief Get local coordinate of the position and rotation center in global frame
         * @note It can be a bit counter intuitive that this is not usually the origin, neither the geometric center of the
         * model, but the geometric center of the sensitive part. This way, the position of the sensing element is invariant
         * under rotations
         *
         * The center coordinate corresponds to the \ref Detector::getPosition "position" in the global frame.
         */
        virtual ROOT::Math::XYZPoint getMatrixCenter() const {
            return {getMatrixSize().x() / 2.0 - getPixelSize().x() / 2.0,
                    getMatrixSize().y() / 2.0 - getPixelSize().y() / 2.0,
                    0};
        }

        /**
         * @brief Get local coordinate of the geometric center of the model
         * @note This returns the center of the geometry model, i.e. including all support layers, passive readout chips et
         * cetera.
         */
        virtual ROOT::Math::XYZPoint getModelCenter() const;

        /**
         * @brief Get size of the wrapper box around the model that contains all elements
         * @return Size of the detector model
         *
         * All elements of the model are covered by a box centered around \ref DetectorModel::getModelCenter. This
         * means that the extend of the model should be calculated using the geometrical center as reference, not the
         * position returned by \ref DetectorModel::getMatrixCenter.
         */
        virtual ROOT::Math::XYZVector getSize() const;

        /* PIXEL GRID */
        /**
         * @brief Get number of pixels (replicated blocks in generic sensors)
         * @return Number of two dimensional pixels
         */
        ROOT::Math::DisplacementVector2D<ROOT::Math::Cartesian2D<unsigned int>> getNPixels() const {
            return number_of_pixels_;
        }
        /**
         * @brief Set number of pixels (replicated blocks in generic sensors)
         * @param val Number of two dimensional pixels
         */
        void setNPixels(ROOT::Math::DisplacementVector2D<ROOT::Math::Cartesian2D<unsigned int>> val) {
            number_of_pixels_ = std::move(val);
        }
        /**
         * @brief Get size of a single pixel
         * @return Size of a pixel
         */
        ROOT::Math::XYVector getPixelSize() const { return pixel_size_; }
        /**
         * @brief Get type of the pixels
         * @return TYpe of the pixels indicating their shape
         */
        Pixel::Type getPixelType() const { return pixel_type_; }

        /**
         * @brief Set the size of a pixel
         * @param val Size of a pixel
         */
        void setPixelSize(ROOT::Math::XYVector val) { pixel_size_ = std::move(val); }
        /**
         * @brief Get size of the collection diode
         * @return Size of the collection diode implant
         */
        ROOT::Math::XYZVector getImplantSize() const { return implant_size_; }
        /**
         * @brief Set size, offset and material of the implant (collection diode) within a pixel
         * @param size Size of the collection diode implant
         * @param offset Offset of the collection diode implant
         * @param material Implant material
         */
        void setImplant(ROOT::Math::XYZVector size, const ROOT::Math::XYVector& offset, std::string material);
        /**
         * @brief Get the material of the implants (collection electrodes)
         * @return Implant material
         */
        virtual std::string getImplantMaterial() const { return implant_material_; }
        /**
         * @brief Get offset of the collection diode from the pixel center
         * This returns the offset of the implant volume from the pixel volume. The surface of the implant is the surface of
         * the pixel cell / the sensor.
         * @return Offset of the collection diode implant
         */
        virtual ROOT::Math::XYZVector getImplantOffset() const { return implant_offset_; }

        /**
         * @brief Get total size of the pixel grid
         * @return Size of the pixel grid
         *
         * @warning The grid has zero thickness
         * @note This is basically a 2D method, but provided in 3D because it is primarily used there
         */
        virtual ROOT::Math::XYZVector getMatrixSize() const {
            return {getNPixels().x() * getPixelSize().x(), getNPixels().y() * getPixelSize().y(), 0};
        }

        /* SENSOR */
        /**
         * @brief Get size of the sensor
         * @return Size of the sensor
         *
         * Calculated from \ref DetectorModel::getMatrixSize "pixel grid size", sensor excess and sensor thickness
         */
        virtual ROOT::Math::XYZVector getSensorSize() const {
            ROOT::Math::XYZVector excess_thickness((sensor_excess_.at(1) + sensor_excess_.at(3)),
                                                   (sensor_excess_.at(0) + sensor_excess_.at(2)),
                                                   sensor_thickness_);
            return getMatrixSize() + excess_thickness;
        }
        /**
         * @brief Get center of the sensor in local coordinates
         * @return Center of the sensor
         *
         * Center of the sensor with excess taken into account
         */
        virtual ROOT::Math::XYZPoint getSensorCenter() const {
            ROOT::Math::XYZVector offset(
                (sensor_excess_.at(1) - sensor_excess_.at(3)) / 2.0, (sensor_excess_.at(0) - sensor_excess_.at(2)) / 2.0, 0);
            return getMatrixCenter() + offset;
        }
        /**
         * @brief Get the material of the sensor
         * @return Material of the sensor
         */
        SensorMaterial getSensorMaterial() { return sensor_material_; }
        /**
         * @brief Set the thickness of the sensor
         * @param val Thickness of the sensor
         */
        void setSensorThickness(double val) { sensor_thickness_ = val; }
        /**
         * @brief Set the excess at the top of the sensor (positive y-coordinate)
         * @param val Sensor top excess
         */
        void setSensorExcessTop(double val) { sensor_excess_.at(0) = val; }
        /**
         * @brief Set the excess at the right of the sensor (positive x-coordinate)
         * @param val Sensor right excess
         */
        void setSensorExcessRight(double val) { sensor_excess_.at(1) = val; }
        /**
         * @brief Set the excess at the bottom of the sensor (negative y-coordinate)
         * @param val Sensor bottom excess
         */
        void setSensorExcessBottom(double val) { sensor_excess_.at(2) = val; }
        /**
         * @brief Set the excess at the left of the sensor (negative x-coordinate)
         * @param val Sensor right excess
         */
        void setSensorExcessLeft(double val) { sensor_excess_.at(3) = val; }

        /* CHIP */
        /**
         * @brief Get size of the chip
         * @return Size of the chip
         *
         * Calculated from \ref DetectorModel::getMatrixSize "pixel grid size", sensor excess and chip thickness
         */
        virtual ROOT::Math::XYZVector getChipSize() const {
            ROOT::Math::XYZVector excess_thickness(
                assembly_->getChipExcess().x(), assembly_->getChipExcess().y(), assembly_->getChipThickness());
            return getMatrixSize() + excess_thickness;
        }
        /**
         * @brief Get center of the chip in local coordinates
         * @return Center of the chip
         *
         * Center of the chip calculcated from chip excess and sensor offset
         */
        virtual ROOT::Math::XYZPoint getChipCenter() const {
            ROOT::Math::XYZVector offset(assembly_->getChipOffset().x() / 2.0,
                                         assembly_->getChipOffset().y() / 2.0,
                                         getSensorSize().z() / 2.0 + getChipSize().z() / 2.0 +
                                             assembly_->getChipOffset().z());
            return getMatrixCenter() + offset;
        }

        /* SUPPORT */
        /**
         * @brief Return all layers of support
         * @return List of all the support layers
         *
         * This method internally computes the correct center of all the supports by stacking them in linear order on both
         * the chip and the sensor side.
         */
        virtual std::vector<SupportLayer> getSupportLayers() const;

        /**
         * @brief Add a new layer of support
         * @param size Size of the support in the x,y-plane
         * @param thickness Thickness of the support
         * @param offset Offset of the support in the x,y-plane
         * @param material Material of the support
         * @param type Type of the hole
         * @param location Location of the support (either 'sensor' or 'chip')
         * @param hole_size Size of the optional hole in the support
         * @param hole_offset Offset of the hole from its default position
         */
        // FIXME: Location (and material) should probably be an enum instead
        void addSupportLayer(const ROOT::Math::XYVector& size,
                             double thickness,
                             ROOT::Math::XYZVector offset,
                             std::string material,
                             std::string type,
                             std::string location,
                             const ROOT::Math::XYVector& hole_size,
                             ROOT::Math::XYVector hole_offset) {
            ROOT::Math::XYZVector full_size(size.x(), size.y(), thickness);
            ROOT::Math::XYZVector full_hole_size(hole_size.x(), hole_size.y(), thickness);
            support_layers_.push_back(SupportLayer(full_size,
                                                   std::move(offset),
                                                   std::move(material),
                                                   std::move(type),
                                                   std::move(location),
                                                   full_hole_size,
                                                   std::move(hole_offset)));
        }

        /**
         * @brief Returns if a local position is within the sensitive device
         * @param local_pos Position in local coordinates of the detector model
         * @return True if a local position is within the sensor, false otherwise
         *
         * @note This method is purely virtual and must be implemented by the respective concrete detector model classes
         */
        virtual bool isWithinSensor(const ROOT::Math::XYZPoint& local_pos) const = 0;

        /**
         * @brief Calculate exit point of step outside sensor volume from one point inside the sensor (before step) and one
         * point outside (after step).
         * @throws std::invalid_argument if no intersection of track segment with sensor volume can be found
         * @param  inside Position before the step, inside the sensor volume
         * @param  outside  Position after the step, outside the sensor volume
         * @return Exit point of the sensor in local coordinates
         *
         * @note This method is purely virtual and must be implemented by the respective concrete detector model classes
         */
        virtual ROOT::Math::XYZPoint getSensorIntercept(const ROOT::Math::XYZPoint& inside,
                                                        const ROOT::Math::XYZPoint& outside) const = 0;

        /**
         * @brief Returns if a local position is within the pixel implant region of the sensitive device
         * @param local_pos Position in local coordinates of the detector model
         * @return True if a local position is within the pixel implant, false otherwise
         *
         * @note This method is purely virtual and must be implemented by the respective concrete detector model classes
         */
        virtual bool isWithinImplant(const ROOT::Math::XYZPoint& local_pos) const = 0;

        /**
         * @brief Calculate entry point of step into impant volume from one point outside the implant (before step) and one
         * point inside (after step).
         * @throws std::out_of_range if no intersection of track segment with implant volume can be found
         * @param  previous Position before the step
         * @param  current  Position after the step
         * @return Entry point in implant in local coordinates of the sensor
         */
        ROOT::Math::XYZPoint getImplantEntry(const ROOT::Math::XYZPoint previous, const ROOT::Math::XYZPoint current) const;

        /**
         * @brief Returns if a pixel index is within the grid of pixels defined for the device
         * @param pixel_index Pixel index to be checked
         * @return True if pixel_index is within the pixel grid, false otherwise
         *
         * @note This method is purely virtual and must be implemented by the respective concrete detector model classes
         */
        virtual bool isWithinMatrix(const Pixel::Index& pixel_index) const = 0;

        /**
         * @brief Returns if a set of pixel coordinates is within the grid of pixels defined for the device
         * @param x X- (or column-) coordinate to be checked
         * @param y Y- (or row-) coordinate to be checked
         * @return True if pixel coordinates are within the pixel grid, false otherwise
         *
         * @note This method is purely virtual and must be implemented by the respective concrete detector model classes
         */
        virtual bool isWithinMatrix(const int x, const int y) const = 0;

        /**
         * @brief Returns if a position is within the grid of pixels defined for the device
         * @param position Position in local coordinates of the detector model
         * @return True if position within the pixel grid, false otherwise
         */
        bool isWithinMatrix(const ROOT::Math::XYZPoint& position) const {
            auto [index_x, index_y] = getPixelIndex(position);
            return isWithinMatrix(index_x, index_y);
        }

        /**
         * @brief Returns a pixel center in local coordinates
         * @param x X- (or column-) coordinate of the pixel
         * @param y Y- (or row-) coordinate of the pixel
         * @return Coordinates of the pixel center
         *
         * @note This method is purely virtual and must be implemented by the respective concrete detector model classes
         */
        virtual ROOT::Math::XYZPoint getPixelCenter(const int x, const int y) const = 0;
        /**
         * @brief Return X,Y indices of a pixel corresponding to a local position in a sensor.
         * @param local_pos Position in local coordinates of the detector model
         * @return X,Y pixel indices
         *
         * @note No checks are performed on whether these indices represent an existing pixel or are within the pixel matrix.
         *
         * @note This method is purely virtual and must be implemented by the respective concrete detector model classes
         */
        virtual std::pair<int, int> getPixelIndex(const ROOT::Math::XYZPoint& local_pos) const = 0;

        /**
         * @brief Return a set containing all pixels neighboring the given one with a configurable maximum distance
         * @param idx       Index of the pixel in question
         * @param distance  Distance for pixels to be considered neighbors
         * @return Set of neighboring pixel indices, including the initial pixel
         *
         * @note The returned set should always also include the initial pixel indices the neighbors are calculated for
         *
         * @note This method is purely virtual and must be implemented by the respective concrete detector model classes
         */
        virtual std::set<Pixel::Index> getNeighbors(const Pixel::Index& idx, const size_t distance) const = 0;

        /**
         * @brief Check if two pixel indices are neighbors to each other
         * @param  seed    Initial pixel index
         * @param  entrant Entrant pixel index to be tested
         * @param distance  Distance for pixels to be considered neighbors
         * @return         Boolean whether pixels are neighbors or not
         *
         * @note This method is purely virtual and must be implemented by the respective concrete detector model classes
         */
        virtual bool areNeighbors(const Pixel::Index& seed, const Pixel::Index& entrant, const size_t distance) const = 0;

    protected:
        /**
         * @brief Liang–Barsky clipping of a line against faces of a box. See Liang, Y. D., and Barsky, B., "A New Concept
         * and Method for Line Clipping", ACM Transactions on Graphics, 3(1):1–22 for an in-depth explanation
         * @param denominator Denominator representing the currently tested face of the box
         * @param numerator Numerator representing the currently tested face of the box
         * @param t0 Line scale of first possible intersection
         * @param t1 Line scale of second possible intersection
         */
        static bool liang_barsky_clipping(double denominator, double numerator, double& t0, double& t1);

        std::string type_;

        ROOT::Math::DisplacementVector2D<ROOT::Math::Cartesian2D<unsigned int>> number_of_pixels_;
        ROOT::Math::XYVector pixel_size_;
        ROOT::Math::XYZVector implant_size_;
        ROOT::Math::XYZVector implant_offset_;
        std::string implant_material_;
        Pixel::Type pixel_type_{Pixel::Type::RECTANGLE};

        double sensor_thickness_{};
        std::array<double, 4> sensor_excess_{};
        SensorMaterial sensor_material_{SensorMaterial::SILICON};

        std::shared_ptr<DetectorAssembly> assembly_;
        std::vector<SupportLayer> support_layers_;

    private:
        ConfigReader reader_;
    };
} // namespace allpix

#endif // ALLPIX_DETECTOR_MODEL_H
