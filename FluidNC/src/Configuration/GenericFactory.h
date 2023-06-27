// Copyright (c) 2021 -	Stefan de Bruijn
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include <vector>
#include "../Logging.h"

#include "HandlerBase.h"

namespace Configuration {
    template <typename BaseType>
    class GenericFactory {
        static GenericFactory& instance() {
            static GenericFactory instance_;
            return instance_;
        }

        GenericFactory() = default;

        GenericFactory(const GenericFactory&)            = delete;
        GenericFactory& operator=(const GenericFactory&) = delete;

        class BuilderBase {
            const char* name_;

        public:
            BuilderBase(const char* name) : name_(name) {}

            BuilderBase(const BuilderBase& o)            = delete;
            BuilderBase& operator=(const BuilderBase& o) = delete;

            virtual std::unique_ptr<BaseType> create() const = 0;
            const char*                       name() const { return name_; }

            virtual ~BuilderBase() = default;
        };

        std::vector<BuilderBase*> builders_;

        inline static void registerBuilder(BuilderBase* builder) { instance().builders_.push_back(builder); }

    public:
        template <typename DerivedType>
        class InstanceBuilder : public BuilderBase {
        public:
            InstanceBuilder(const char* name) : BuilderBase(name) { instance().registerBuilder(this); }

            std::unique_ptr<BaseType> create() const override { return std::make_unique<DerivedType>(); }
        };

        static void factory(Configuration::HandlerBase& handler, std::unique_ptr<BaseType>& inst) {
            if (inst == nullptr) {
                for (auto it : instance().builders_) {
                    if (handler.matchesUninitialized(it->name())) {
                        inst = it->create();
                        handler.enterFactory(it->name(), *inst);

                        return;
                    }
                }
            } else {
                handler.enterSection(inst->name(), inst.get());
            }
        }

        static void factory(Configuration::HandlerBase& handler, std::vector<std::unique_ptr<BaseType>>& insts) {
            if (handler.handlerType() == HandlerType::Parser) {
                for (auto builder : instance().builders_) {
                    if (handler.matchesUninitialized(builder->name())) {
                        auto inst = builder->create();
                        handler.enterFactory(builder->name(), *inst);
                        insts.push_back(std::move(inst));
                        return;
                    }
                }
            } else {
                for (auto& inst : insts) {
                    handler.enterSection(inst->name(), inst.get());
                }
            }
        }
    };
}
