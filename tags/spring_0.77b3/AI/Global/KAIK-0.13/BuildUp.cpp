#include "BuildUp.h"


CR_BIND(CBuildUp, (NULL));
CR_REG_METADATA(CBuildUp, (
	CR_MEMBER(ai),
	CR_MEMBER(factoryTimer),
	CR_MEMBER(builderTimer),
	CR_MEMBER(storageTimer),
	CR_MEMBER(nukeSiloTimer),
	CR_RESERVED(16)
));


CBuildUp::CBuildUp(AIClasses* ai) {
	this->ai = ai;

	// these are used to determine how many update
	// cycles should pass before building or unit of
	// this type is (re-)considered for construction
	factoryTimer = 0;
	builderTimer = 1;
	storageTimer = 0;
	nukeSiloTimer = 0;
}
CBuildUp::~CBuildUp() {
}


void CBuildUp::Update(int frame) {
	if (frame % 15 == 0) {
		// update current threat map
		ai->tm->Create();
		Buildup(frame);

		// KLOOTNOTE: b1 will be false if we
		// have huge amounts of metal storage,
		// so make multiplier variable (more
		// or less assumes a starting storage
		// capacity of 1000)
		float m = 900.0f / (ai->cb->GetMetalStorage());
		bool b1 = (ai->cb->GetMetal()) > (ai->cb->GetMetalStorage() * m);
		bool b2 = (ai->cb->GetEnergyIncome()) > (ai->cb->GetEnergyUsage() * 1.3f);
		bool b3 = (ai->cb->GetMetalIncome()) > (ai->cb->GetMetalUsage() * 1.3f);

		if ((b1 && b2 && b3) && builderTimer > 0 && !(rand() % 3) && frame > 3600) {
			// decrease builderTime iif we have more metal
			// than 90% of our metal storage capacity and
			// we are generating more than 130% the amount
			// of M and E used (meaning we have excess M
			// and are over-producing M and E)
			builderTimer--;
			factoryTimer--;
		}

		if (storageTimer > 0)
			storageTimer--;

		if (nukeSiloTimer > 0)
			nukeSiloTimer--;
	}
}




void CBuildUp::Buildup(int frame) {
	// KLOOTNOTE: make it emphasize mexes and energy more
	// during the first two minutes (important for PURE)
	float mIncome = ai->cb->GetMetalIncome();
	float eIncome = ai->cb->GetEnergyIncome();
	float mLevel = ai->cb->GetMetal();
	float eLevel = ai->cb->GetEnergy();
	float mStorage = ai->cb->GetMetalStorage();
	float eStorage = ai->cb->GetEnergyStorage();
	float mUsage = ai->cb->GetMetalUsage();
	float eUsage = ai->cb->GetEnergyUsage();
	bool makersOn = ai->uh->metalMaker->AllAreOn();

	float m1 = 500.0f / mStorage;					// 0.5f
	float m2 = 200.0f / mStorage;					// 0.2f
	float e1 = 500.0f / eStorage;					// 0.5f
	float e2 = 800.0f / eStorage;					// 0.8f
	bool mLevel50 = (mLevel < (mStorage * m1));		// is our current metal level less than 50% of our current metal storage capacity?
	bool eLevel50 = (eLevel > (eStorage * e1));		// is our current energy level more than 50% of our current energy storage capacity?
	bool eLevel80 = (eLevel > (eStorage * e2));		// is our current energy level more than 80% of our current energy storage capacity?

	// fake a resource crisis during the first
	// minute to get our economy going quicker
	// KLOOTNOTE: reverted, has opposite effect
	bool mStall = (/*(frame < 1800) ||*/ (mIncome < (mUsage * 1.3f)));	// are we currently producing less metal than we are currently expending * 1.3?
	bool eStall = (/*(frame <  900) ||*/ (eIncome < (eUsage * 1.6f)));	// are we currently producing less energy than we are currently expending * 1.6?


	// KLOOTNOTE: <MAX_NUKE_SILOS> nuke silos ought to be enough for
	// everybody (assuming we can build them at all in current mod)
	// TODO: use actual metal and energy drain of nuke weapon here
	bool buildNukeSilo =
		(mIncome > 100.0f && eIncome > 6000.0f && mUsage < mIncome && eUsage < eIncome &&
		ai->ut->nuke_silos->size() > 0 && ai->uh->NukeSilos.size() < MAX_NUKE_SILOS);


	if (ai->uh->NumIdleUnits(CAT_BUILDER)) {
		// get first idle (mobile) builder every Update() cycle
		int builder = ai->uh->GetIU(CAT_BUILDER);
		const UnitDef* builderDef = ai->cb->GetUnitDef(builder);
		const UnitDef* factoryDef = ai->ut->GetUnitByScore(builder, CAT_FACTORY);

		// if this builder cannot build any factories, pretend it's feasible
		bool factFeasM = (factoryDef? ai->math->MFeasibleConstruction(builderDef, factoryDef): true);
		bool factFeasE = (factoryDef? ai->math->EFeasibleConstruction(builderDef, factoryDef): true);
		bool factFeas = (factoryDef && factFeasM && factFeasE);
		bool eLevelMed = (eLevel50 && makersOn);
		bool mLevelLow = (mLevel50 || (((RANDINT % 3) == 0) && mStall && eLevel80) || (!factFeasM && factoryTimer <= 0));

		// number of buildings in unit-table, not how many currently built
		int buildableEStorage = ai->ut->energy_storages->size();
		int buildableMMakers = ai->ut->metal_makers->size();


		if (!builderDef) {
			ai->uh->UnitDestroyed(builder);
		}

		else {
			if (builderDef->isCommander) {
				if (builderDef->canDGun && ai->dgunController->isBusy()) {
					// don't start building solars etc. while dgun-controller is doing stuff
					return;
				} else {
					int numM = ai->uh->AllUnitsByCat[CAT_MEX].size();
					int numE = ai->uh->AllUnitsByCat[CAT_ENERGY].size();
					int numF = ai->uh->AllUnitsByCat[CAT_FACTORY].size();

					// note: in EE metal processors belong to CAT_ENERGY
					if ((numM < 3 && numE <= 3) && BuildUpgradeExtractor(builder)) { return; }
					if ((numE < 3 && numM <= 3) && BuildUpgradeReactor(builder)) { return; }
					if ((numF < 1 && factFeas) && BuildNow(builder, CAT_FACTORY, factoryDef)) { return; }

					if (ai->uh->FactoryBuilderAdd(builder)) {
						// add commander to factory so it doesn't wander around too much (works best if
						// AI given bonus, otherwise initial expansion still mostly done by commander)
						// note: 5 minutes should be enough to get the resource income needed for this,
						// don't use hardcoded metal- and energy-values
						builderTimer = 0;
					}
				}
			}


			else if (buildNukeSilo && nukeSiloTimer <= 0) {
				if (!ai->uh->BuildTaskAddBuilder(builder, CAT_NUKE)) {
					// always favor building one silo at a time rather than
					// many in parallel to prevent a sudden massive resource
					// drain when silos are finished
					if (BuildNow(builder, CAT_NUKE))
						nukeSiloTimer += 300;
				}
			}


			else if (eLevelMed && mLevelLow) {
				// only reclaim features during odd frames so we don't
				// spend the entire game just chasing after rocks etc.
				// (problem on Cooper Hill and similar maps)
				bool reclaimFeature = ((frame & 1) && ai->MyUnits[builder]->ReclaimBestFeature(true));

				if (!reclaimFeature) {
					bool b = BuildUpgradeExtractor(builder);
					bool eOverflow = (eStorage / (eIncome + 0.01) < STORAGETIME);
					bool eExcess = (eIncome > (eUsage * 1.5));

					// if we couldn't build or upgrade an extractor
					if (!b && eOverflow && buildableEStorage > 0 && storageTimer <= 0) {
						if (!ai->uh->BuildTaskAddBuilder(builder, CAT_ESTOR)) {
							// build energy storage
							if (BuildNow(builder, CAT_ESTOR))
								storageTimer += 90;
						}
					}
					else if (!b && buildableMMakers > 0 && eExcess && ((RANDINT % 10) == 0)) {
						// build metal maker
						if (!ai->uh->BuildTaskAddBuilder(builder, CAT_MMAKER)) {
							BuildNow(builder, CAT_MMAKER);
						}
					}
				}
			}


			// we're producing lots of energy but aren't using it
			else if (eIncome > 2000.0f && eUsage < (eIncome - 1000.0f) && (mStall && mLevel < 100.0f) && buildableMMakers > 0) {
				if (!ai->uh->BuildTaskAddBuilder(builder, CAT_MMAKER)) {
					BuildNow(builder, CAT_MMAKER);
				}
			}


			else if (eStall || !factFeasE) {
				// build energy generator
				if (!ai->uh->BuildTaskAddBuilder(builder, CAT_ENERGY)) {
					BuildUpgradeReactor(builder);
				}
			}

			else {
				bool mOverflow = (mStorage / (mIncome + 0.01)) < (STORAGETIME * 2);
				bool numMStorage = ai->ut->metal_storages->size();
				int numDefenses = ai->uh->AllUnitsByCat[CAT_DEFENCE].size();
				int numFactories = ai->uh->AllUnitsByCat[CAT_FACTORY].size();

				// do we have more factories than defense (and have at least 10 minutes passed)?
				if (numFactories > (numDefenses / DEFENSEFACTORYRATIO) && frame > 18000) {
					if (mOverflow && numMStorage > 0 && storageTimer <= 0 && (numFactories > 0)) {
						if (!ai->uh->BuildTaskAddBuilder(builder, CAT_MSTOR)) {
							// build metal storage
							if (BuildNow(builder, CAT_MSTOR))
								storageTimer += 90;
						}
					} else {
						if (!ai->uh->BuildTaskAddBuilder(builder, CAT_DEFENCE)) {
							// if we can't add this builder to some defense
							// task then build something in CAT_DEFENCE
							const UnitDef* building = ai->ut->GetUnitByScore(builder, CAT_DEFENCE);
							bool r = false;

							if (building) {
								float3 buildPos = ai->dm->GetDefensePos(building, ai->MyUnits[builder]->pos());
								r = ai->MyUnits[builder]->Build_ClosestSite(building, buildPos, 2);
							} else {
								FallbackBuild(builder, CAT_DEFENCE);
							}
						}
					}
				}

				// no, build more factories
				else {
					if (!ai->uh->BuildTaskAddBuilder(builder, CAT_FACTORY)) {
						// if we can't add this builder to some other buildtask
						if (!ai->uh->FactoryBuilderAdd(builder)) {
							// if we can't add this builder to some
							// other factory then construct new one
							if (ai->uh->AllUnitsByCat[CAT_FACTORY].size() < 1 || frame > 9000) {
								// one factory for the first 5 minutes
								BuildNow(builder, CAT_FACTORY, factoryDef);
							}
						}
					}
				}
			}
		}
	}


	bool b1 = ((eLevel > (eStorage * e2)) || (eIncome > 6000.0f && eUsage < eIncome));
	bool b2 = ((mLevel > (mStorage * m2)) || (mIncome > 100.0f && mUsage < mIncome));

	if (b1 && b2) {
		FactoryCycle(frame);
	}

	if (buildNukeSilo) {
		NukeSiloCycle();
	}
}




void CBuildUp::FactoryCycle(int frame) {
	int numIdleFactories = ai->uh->NumIdleUnits(CAT_FACTORY);

	for (int i = 0; i < numIdleFactories; i++) {
		// pick the i-th idle factory we have
		int producedCat = LASTCATEGORY;
		int factoryUnitID = ai->uh->GetIU(CAT_FACTORY);
		bool isHub = (ai->MyUnits[factoryUnitID]->isHub());
		const UnitDef* factDef = ai->MyUnits[factoryUnitID]->def();

		// assume that factories with tech-level TL > 0 are
		// useful to keep active and building for (TL * 30)
		// minutes, but depreciate rapidly after that point
		// TODO: don't reduce factory build frequency, but
		// focus more on mobile constructors instead?
		int tchLvl = ai->ut->unitTypes[factDef->id].techLevel;
		bool obsolete = ((tchLvl > 0)? ((tchLvl * 30) > (frame / 1800)): false);
		bool mayBuild = ((obsolete)? (frame % 1800 == 0): true);

		if (mayBuild) {
			if (isHub) {
				// if we are a hub then assume we can only construct
				// factories and some other types of static buildings
				// note: not always true, in Evolution the "commander"
				// (starting factory unit) can construct mobile units!
				if (factDef->isCommander) {
					producedCat = CAT_BUILDER;
					builderTimer = 0;
				} else {
					producedCat = CAT_FACTORY;
					factoryTimer = 0;
				}
			}
			else {
				if ((builderTimer > 0) || (ai->uh->NumIdleUnits(CAT_BUILDER) > 2)) {
					// if we have more than two idle builders
					// then compensate with an offensive unit
					producedCat = CAT_G_ATTACK;
					builderTimer = std::max(0, builderTimer - 1);
				}

				else {
					const UnitDef* leastBuiltBuilder = GetLeastBuiltBuilder();
					const UnitDef* builderUnit = ai->ut->GetUnitByScore(factoryUnitID, CAT_BUILDER);

					if (builderUnit && builderUnit == leastBuiltBuilder) {
						// if this factory makes the builder that we are short of
						producedCat = CAT_BUILDER;
						builderTimer += 4;
					} else {
						// build some offensive unit
						producedCat = CAT_G_ATTACK;
						builderTimer = std::max(0, builderTimer - 1);
					}
				}
			}

			// get a unit of the category we want this factory to produce
			const UnitDef* udef = ai->ut->GetUnitByScore(factoryUnitID, producedCat);

			if (udef) {
				if (isHub) {
					bool factFeasM = ai->math->MFeasibleConstruction(factDef, udef);
					bool factFeasE = ai->math->EFeasibleConstruction(factDef, udef);
					bool factFeas = factFeasM && factFeasE;
					if (factFeas) {
						(ai->MyUnits[factoryUnitID])->HubBuild(udef);
					}
				} else {
					(ai->MyUnits[factoryUnitID])->FactoryBuild(udef);
				}
			}
		}
	}
}


// queue up nukes if we have any silos (note that this
// doesn't cause a resource drain if silo still under
// construction, missiles won't start building until
// silo done)
void CBuildUp::NukeSiloCycle(void) {
	for (std::list<NukeSilo>::iterator i = ai->uh->NukeSilos.begin(); i != ai->uh->NukeSilos.end(); i++) {
		NukeSilo* silo = &*i;
		ai->cb->GetProperty(silo->id, AIVAL_STOCKPILED, &(silo->numNukesReady));
		ai->cb->GetProperty(silo->id, AIVAL_STOCKPILE_QUED, &(silo->numNukesQueued));

		// always keep at least 5 nukes in queue for a rainy day
		if (silo->numNukesQueued < 5)
			ai->MyUnits[silo->id]->NukeSiloBuild();
	}
}




void CBuildUp::FallbackBuild(int builder, int failedCat) {
	// called if an idle builder was selected to construct
	// some category of unit, but builder not capable of
	// constructing anything of that category (note that
	// if AI is swimming in resources then most L1 builders
	// will be used in assisting roles)

	bool b1 = ai->uh->BuildTaskAddBuilder(builder, CAT_MEX);
	bool b2 = false;
	bool b3 = false;
	bool b4 = false;
	float3 builderPos = ai->cb->GetUnitPos(builder);

	if (!b1              ) { b2 = ai->uh->BuildTaskAddBuilder(builder, CAT_ENERGY); }
	if (!b1 && !b2       ) { b3 = ai->uh->BuildTaskAddBuilder(builder, CAT_DEFENCE); }
	if (!b1 && !b2 && !b3) { b4 = ai->uh->BuildTaskAddBuilder(builder, CAT_FACTORY); }

/*
	if (!b1 && !b2 && !b3 && !b4) {
		// failed to add builder to any task, try building something
		const UnitDef* udef1 = ai->ut->GetUnitByScore(builder, CAT_MEX);
		const UnitDef* udef2 = ai->ut->GetUnitByScore(builder, CAT_ENERGY);
		const UnitDef* udef3 = ai->ut->GetUnitByScore(builder, CAT_DEFENCE);
		const UnitDef* udef4 = ai->ut->GetUnitByScore(builder, CAT_FACTORY);

		if (udef2 && failedCat != CAT_ENERGY) {
			ai->MyUnits[builder]->Build_ClosestSite(udef2, builderPos);
			return;
		}
		if (udef3 && failedCat != CAT_DEFENCE) {
			float3 pos = ai->dm->GetDefensePos(udef3, builderPos);
			ai->MyUnits[builder]->Build_ClosestSite(udef3, pos);
			return;
		}
		if (udef4 && failedCat != CAT_FACTORY) {
			ai->MyUnits[builder]->Build_ClosestSite(udef4, builderPos);
			return;
		}
		if (udef1 && failedCat != CAT_MEX) {
			float3 pos = ai->mm->GetNearestMetalSpot(builder, udef1);
			if (pos != ERRORVECTOR)
				ai->MyUnits[builder]->Build(pos, udef1, -1);
			return;
		}
	}
*/

	// unable to assist and unable to build, just patrol
	if (!b1 && !b2 && !b3 && !b4)
		ai->MyUnits[builder]->Patrol(builderPos);
}



// look at all online factories and their best builders,
// then find the best builder that there are least of
const UnitDef* CBuildUp::GetLeastBuiltBuilder(void) {
	int factoryCount = ai->uh->AllUnitsByCat[CAT_FACTORY].size();
	const UnitDef* leastBuiltBuilder = 0;
	int leastBuiltBuilderCount = 65536;
	assert(factoryCount > 0);

	for (list<int>::iterator j = ai->uh->AllUnitsByCat[CAT_FACTORY].begin(); j != ai->uh->AllUnitsByCat[CAT_FACTORY].end(); j++) {
		// get factory unitID
		int factoryToLookAt = *j;

		if (!ai->cb->UnitBeingBuilt(factoryToLookAt)) {
			// if factory isn't still under construction
			const UnitDef* bestBuilder = ai->ut->GetUnitByScore(factoryToLookAt, CAT_BUILDER);

			if (bestBuilder) {
				int bestBuilderCount = ai->uh->AllUnitsByType[bestBuilder->id].size();

				if (bestBuilderCount < leastBuiltBuilderCount) {
					leastBuiltBuilderCount = bestBuilderCount;
					leastBuiltBuilder = bestBuilder;
				}
			}
		}
	}

	return leastBuiltBuilder;
}



bool CBuildUp::BuildNow(int builder, int category) {
	const UnitDef* building = ai->ut->GetUnitByScore(builder, category);
	bool r = false;

	if (building) {
		r = ai->MyUnits[builder]->Build_ClosestSite(building, ai->cb->GetUnitPos(builder));
	} else {
		FallbackBuild(builder, category);
	}

	return r;
}

bool CBuildUp::BuildNow(int builder, int category, const UnitDef* udef) {
	bool r = false;

	if (udef) {
		r = ai->MyUnits[builder]->Build_ClosestSite(udef, ai->cb->GetUnitPos(builder));
	} else {
		FallbackBuild(builder, CAT_FACTORY);
	}

	return r;
}



bool CBuildUp::BuildUpgradeExtractor(int builder) {
	const UnitDef* mexDef = ai->ut->GetUnitByScore(builder, CAT_MEX);

	if (mexDef) {
		float3 mexPos = ai->mm->GetNearestMetalSpot(builder, mexDef);

		if (mexPos != ERRORVECTOR) {
			if (!ai->uh->BuildTaskAddBuilder(builder, CAT_MEX)) {
				// build metal extractor
				return (ai->MyUnits[builder]->Build(mexPos, mexDef, -1));
			}
		} else {
			// upgrade existing mex (NOTE: GetNearestMetalSpot()
			// very rarely returns an error-vector, needs more
			// incentives)
			int oldMexID = ai->uh->GetOldestMetalExtractor();
			const UnitDef* oldMex = ai->cb->GetUnitDef(oldMexID);

			if (oldMex) {
				if ((mexDef->extractsMetal / oldMex->extractsMetal) >= 2.0f) {
					return (ai->MyUnits[builder]->Upgrade(oldMexID, mexDef));
				}
			}
		}
	}

	// can't build or upgrade
	return false;
}



bool CBuildUp::BuildUpgradeReactor(int builder) {
	const UnitDef* reactorDef = ai->ut->GetUnitByScore(builder, CAT_ENERGY);

	if (reactorDef) {
		const float3 builderPos = ai->cb->GetUnitPos(builder);
		float netEnergy = reactorDef->energyMake - reactorDef->energyUpkeep;

		float closestDstSq = 999999999999999.0f;
		int bestItReactor = -1;
		const UnitDef* bestItReactorDef = 0x0;

		std::list<int> lst = ai->uh->AllUnitsByCat[CAT_ENERGY];

		for (std::list<int>::iterator it = lst.begin(); it != lst.end(); it++) {
			const float3 itReactorPos = ai->cb->GetUnitPos(*it);
			const UnitDef* itReactorDef = ai->cb->GetUnitDef(*it);

			if (itReactorDef->energyMake <= 0.0f || itReactorDef->windGenerator) {
				// only look at solars and windmills which produce energy through
				// negative upkeep (something of a hack for OTA-style mods)
				const float itNetEnergy = itReactorDef->energyMake - itReactorDef->energyUpkeep;
				const float distanceSq = (itReactorPos - builderPos).SqLength();

				// find the closest CAT_ENERGY structure we have
				if (distanceSq < closestDstSq) {
					// check if it is worth upgrading
					if ((netEnergy / itNetEnergy) >= 2.0f) {
						bestItReactor = *it;
						bestItReactorDef = itReactorDef;
					}
				}
			}
		}

		if (bestItReactor != -1) {
			// upgrade
			return (ai->MyUnits[builder]->Upgrade(bestItReactor, reactorDef));
		} else {
			// nothing to upgrade
			return BuildNow(builder, 0, reactorDef);
		}
	}

	// can't build or upgrade
	return false;
}
