// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/CCopasiSimpleSelectionTree.cpp,v $
//   $Revision: 1.30 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/02/19 19:53:07 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include "copasi.h"

#include "CCopasiSimpleSelectionTree.h"
#include "model/CModel.h"
#include "model/CMetab.h"
#include "model/CReaction.h"
#include "utilities/CCopasiParameter.h"
#include "utilities/CCopasiParameterGroup.h"
#include "report/CCopasiObject.h"
#include "report/CCopasiContainer.h"
#include "report/CCopasiTimer.h"
#include "report/CCopasiObjectName.h"
#include "qtUtilities.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "utilities/CAnnotatedMatrix.h"
#include "utilities/CCopasiTask.h"
#include "steadystate/CMCAMethod.h"
#include "steadystate/CSteadyStateTask.h"
#include "sensitivities/CSensProblem.h"

CCopasiSimpleSelectionTree::CCopasiSimpleSelectionTree(QWidget* parent, const char* name, Qt::WFlags fl): Q3ListView(parent, name, fl), mpOutputVector(NULL)
{
  setSelectionMode(Q3ListView::Extended);
  setSortColumn(-1);
  setRootIsDecorated(true);
  addColumn("");
  setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth()));
  mpExpertSubtree = new Q3ListViewItem(this, "Expert");

  mpResultMatrixSubtree = new Q3ListViewItem(this, "Results");
  mpResultSteadyStateSubtree = new Q3ListViewItem(mpResultMatrixSubtree, "Steady State");
  mpResultSensitivitySubtree = new Q3ListViewItem(mpResultMatrixSubtree, "Sensitivity");
  mpResultMCASubtree = new Q3ListViewItem(mpResultMatrixSubtree, "Metabolic Control Analysis");

  mpModelMatrixSubtree = new Q3ListViewItem(this, "Matrices");

  mpModelQuantitySubtree = new Q3ListViewItem(this, "Global Quantities");
  mpModelQuantityRateSubtree =
    new Q3ListViewItem(mpModelQuantitySubtree, "Rates");
  mpModelQuantityTransientValueSubtree =
    new Q3ListViewItem(mpModelQuantitySubtree, "Transient Values");
  mpModelQuantityInitialValueSubtree =
    new Q3ListViewItem(mpModelQuantitySubtree, "Initial Values");

  mpReactionSubtree = new Q3ListViewItem(this, "Reactions");
  mpReactionFluxNumberSubtree =
    new Q3ListViewItem(mpReactionSubtree, "Fluxes (Particle Numbers)");
  mpReactionFluxConcentrationSubtree =
    new Q3ListViewItem(mpReactionSubtree, "Fluxes (Concentration)");
  mpReactionParameterSubtree =
    new Q3ListViewItem(mpReactionSubtree, "Reaction Parameters");

  mpMetaboliteSubtree = new Q3ListViewItem(this, "Species");
  mpMetaboliteRateNumberSubtree =
    new Q3ListViewItem(mpMetaboliteSubtree, "Rates (Particle Numbers)");
  mpMetaboliteRateConcentrationSubtree =
    new Q3ListViewItem(mpMetaboliteSubtree, "Rates (Concentrations)");
  mpMetaboliteTransientNumberSubtree =
    new Q3ListViewItem(mpMetaboliteSubtree, "Transient Particle Numbers");
  mpMetaboliteTransientConcentrationSubtree =
    new Q3ListViewItem(mpMetaboliteSubtree, "Transient Concentrations");
  mpMetaboliteInitialNumberSubtree =
    new Q3ListViewItem(mpMetaboliteSubtree, "Initial Particle Numbers");
  mpMetaboliteInitialConcentrationSubtree =
    new Q3ListViewItem(mpMetaboliteSubtree, "Initial Concentrations");

  mpCompartmentSubtree = new Q3ListViewItem(this, "Compartments");
  mpCompartmentRateSubtree =
    new Q3ListViewItem(mpCompartmentSubtree, "Rates");
  mpCompartmentTransientVolumeSubtree =
    new Q3ListViewItem(mpCompartmentSubtree, "Transient Volumes");
  mpCompartmentInitialVolumeSubtree =
    new Q3ListViewItem(mpCompartmentSubtree, "Initial Volumes");

  mpTimeSubtree = new Q3ListViewItem(this, "Time");
}

/// Destructor
CCopasiSimpleSelectionTree::~CCopasiSimpleSelectionTree()
{}

/*
 * build the population tree
 */
void CCopasiSimpleSelectionTree::populateTree(const CModel * pModel,
    const SelectionFlag & flag)
{
  if (!pModel) return;

  const CCopasiObject * pObject;
  Q3ListViewItem * pItem;

  // find all kinds of time
  pObject = pModel->getObject(CCopasiObjectName("Reference=Time"));
  if (filter(flag, pObject))
    {
      pItem = new Q3ListViewItem(mpTimeSubtree, "Model Time");
      treeItems[pItem] = pObject;
    }

  pObject = pModel->getObject(CCopasiObjectName("Reference=Initial Time"));
  if (filter(flag, pObject))
    {
      pItem = new Q3ListViewItem(mpTimeSubtree, "Model Initial Time");
      treeItems[pItem] = pObject;
    }

  pObject = pModel->getParentDatamodel()->getObject(CCopasiObjectName("Timer=CPU Time"));
  if (filter(flag, pObject))
    {
      pItem = new Q3ListViewItem(mpTimeSubtree, "CPU time");
      treeItems[pItem] = pObject;
    }

  pObject = pModel->getParentDatamodel()->getObject(CCopasiObjectName("Timer=Wall Clock Time"));
  if (filter(flag, pObject))
    {
      pItem = new Q3ListViewItem(mpTimeSubtree, "real time");
      treeItems[pItem] = pObject;
    }

  removeEmptySubTree(&mpTimeSubtree);

  // find all species (aka metabolites) and create items in the metabolite subtree
  const CCopasiVector<CMetab>& metabolites = pModel->getMetabolites();
  unsigned int counter;
  unsigned int maxCount = metabolites.size();
  for (counter = maxCount; counter != 0;--counter)
    {
      const CMetab* metab = metabolites[counter - 1];
      std::string name = metab->getObjectName();
      bool unique = isMetaboliteNameUnique(name, metabolites);
      if (!unique)
        {
          const CCompartment* comp = metab->getCompartment();
          if (comp)
            {
              name = name + "(" + comp->getObjectName() + ")";
            }
        }

      pObject = metab->getObject(CCopasiObjectName("Reference=InitialParticleNumber"));
      if (filter(flag, pObject))
        {
          pItem = new Q3ListViewItem(mpMetaboliteInitialNumberSubtree, FROM_UTF8(name + "(t=0)"));
          treeItems[pItem] = pObject;
        }

      pObject = metab->getObject(CCopasiObjectName("Reference=ParticleNumber"));
      if (filter(flag, pObject))
        {
          pItem = new Q3ListViewItem(mpMetaboliteTransientNumberSubtree, FROM_UTF8(name + "(t)"));
          treeItems[pItem] = pObject;
        }

      pObject = metab->getObject(CCopasiObjectName("Reference=ParticleNumberRate"));
      if (filter(flag, pObject))
        {
          pItem = new Q3ListViewItem(mpMetaboliteRateNumberSubtree, FROM_UTF8("d(" + name + ")/dt"));
          treeItems[pItem] = pObject;
        }

      name = "[" + name + "]"; // Concentration
      pObject = metab->getObject(CCopasiObjectName("Reference=InitialConcentration"));
      if (filter(flag, pObject))
        {
          pItem = new Q3ListViewItem(mpMetaboliteInitialConcentrationSubtree, FROM_UTF8(name + "(t=0)"));
          treeItems[pItem] = pObject;
        }

      pObject = metab->getObject(CCopasiObjectName("Reference=Concentration"));
      if (filter(flag, pObject))
        {
          pItem = new Q3ListViewItem(mpMetaboliteTransientConcentrationSubtree, FROM_UTF8(name + "(t)"));
          treeItems[pItem] = pObject;
        }

      pObject = metab->getObject(CCopasiObjectName("Reference=Rate"));
      if (filter(flag, pObject))
        {
          pItem = new Q3ListViewItem(mpMetaboliteRateConcentrationSubtree, FROM_UTF8("d(" + name + ")/dt"));
          treeItems[pItem] = pObject;
        }
    }

  removeEmptySubTree(&mpMetaboliteInitialNumberSubtree);
  removeEmptySubTree(&mpMetaboliteTransientNumberSubtree);
  removeEmptySubTree(&mpMetaboliteRateNumberSubtree);
  removeEmptySubTree(&mpMetaboliteInitialConcentrationSubtree);
  removeEmptySubTree(&mpMetaboliteTransientConcentrationSubtree);
  removeEmptySubTree(&mpMetaboliteRateConcentrationSubtree);
  removeEmptySubTree(&mpMetaboliteSubtree);

  // find all reactions and create items in the reaction subtree
  const CCopasiVectorNS<CReaction>& reactions = pModel->getReactions();
  maxCount = reactions.size();
  for (counter = maxCount; counter != 0; --counter)
    {
      const CReaction* react = reactions[counter - 1];
      std::string name = "flux(" + react->getObjectName() + ")";

      pObject = react->getObject(CCopasiObjectName("Reference=Flux"));
      if (filter(flag, pObject))
        {
          pItem = new Q3ListViewItem(mpReactionFluxConcentrationSubtree, FROM_UTF8(name));
          treeItems[pItem] = pObject;
        }

      pObject = react->getObject(CCopasiObjectName("Reference=ParticleFlux"));
      if (filter(flag, pObject))
        {
          pItem = new Q3ListViewItem(mpReactionFluxNumberSubtree, FROM_UTF8("particle " + name));
          treeItems[pItem] = pObject;
        }

      // create items for the reaction parameters

      pItem = new Q3ListViewItem(mpReactionParameterSubtree,
                                 FROM_UTF8(react->getObjectName()));
      const CCopasiParameterGroup & Parameters = react->getParameters();
      unsigned int j;
      unsigned int numParameters = Parameters.size();
      for (j = numParameters; j != 0; --j)
        {
          const CCopasiParameter * pParameter = Parameters.getParameter(j - 1);

          // We skip local parameters which ar covered by global parameters
          if (!react->isLocalParameter(pParameter->getObjectName()))
            continue;

          pObject = pParameter->getObject(CCopasiObjectName("Reference=Value"));
          if (filter(flag, pObject))
            {
              Q3ListViewItem * pParameterItem =
                new Q3ListViewItem(pItem, FROM_UTF8(pParameter->getObjectName()));
              treeItems[pParameterItem] = pObject;
            }
        }

      removeEmptySubTree(&pItem);
    }

  removeEmptySubTree(&mpReactionFluxNumberSubtree);
  removeEmptySubTree(&mpReactionFluxConcentrationSubtree);
  removeEmptySubTree(&mpReactionParameterSubtree);
  removeEmptySubTree(&mpReactionSubtree);

  // find all global parameters (aka model values) variables
  const CCopasiVector<CModelValue>& objects = pModel->getModelValues();
  maxCount = objects.size();
  for (counter = maxCount; counter != 0;--counter)
    {
      const CModelEntity* object = objects[counter - 1];
      std::string name = object->getObjectName();

      pObject = object->getObject(CCopasiObjectName("Reference=InitialValue"));
      if (filter(flag, pObject))
        {
          pItem = new Q3ListViewItem(mpModelQuantityInitialValueSubtree, FROM_UTF8(name + "(t=0)"));
          treeItems[pItem] = pObject;
        }

      pObject = object->getObject(CCopasiObjectName("Reference=Value"));
      if (filter(flag, pObject))
        {
          pItem = new Q3ListViewItem(mpModelQuantityTransientValueSubtree, FROM_UTF8(name + "(t)"));
          treeItems[pItem] = pObject;
        }

      if (object->getStatus() == CModelEntity::ODE)
        {
          pObject = object->getObject(CCopasiObjectName("Reference=Rate"));
          if (filter(flag, pObject))
            {
              pItem = new Q3ListViewItem(mpModelQuantityRateSubtree, FROM_UTF8("d(" + name + ")/dt"));
              treeItems[pItem] = pObject;
            }
        }
    }

  removeEmptySubTree(&mpModelQuantityRateSubtree);
  removeEmptySubTree(&mpModelQuantityInitialValueSubtree);
  removeEmptySubTree(&mpModelQuantityTransientValueSubtree);
  removeEmptySubTree(&mpModelQuantitySubtree);

  // find all compartments
  const CCopasiVector<CCompartment>& objects2 = pModel->getCompartments();
  maxCount = objects2.size();
  for (counter = maxCount; counter != 0;--counter)
    {
      const CModelEntity* object = objects2[counter - 1];
      std::string name = object->getObjectName();

      pObject = object->getObject(CCopasiObjectName("Reference=InitialVolume"));
      if (filter(flag, pObject))
        {
          pItem = new Q3ListViewItem(mpCompartmentInitialVolumeSubtree, FROM_UTF8(name + "(t=0)"));
          treeItems[pItem] = pObject;
        }

      pObject = object->getObject(CCopasiObjectName("Reference=Volume"));
      if (filter(flag, pObject))
        {
          pItem = new Q3ListViewItem(mpCompartmentTransientVolumeSubtree, FROM_UTF8(name + "(t)"));
          treeItems[pItem] = pObject;
        }

      if (object->getStatus() == CModelEntity::ODE)
        {
          pObject = object->getObject(CCopasiObjectName("Reference=Rate"));
          if (filter(flag, pObject))
            {
              pItem = new Q3ListViewItem(mpCompartmentRateSubtree, FROM_UTF8("d(" + name + ")/dt"));
              treeItems[pItem] = pObject;
            }
        }
    }

  removeEmptySubTree(&mpCompartmentRateSubtree);
  removeEmptySubTree(&mpCompartmentInitialVolumeSubtree);
  removeEmptySubTree(&mpCompartmentTransientVolumeSubtree);
  removeEmptySubTree(&mpCompartmentSubtree);

  pObject = pModel->getObject(CCopasiObjectName("Reference=Quantity Conversion Factor"));
  if (filter(flag, pObject))
    {
      pItem = new Q3ListViewItem(this, "Quantity Conversion Factor");
      treeItems[pItem] = pObject;
    }

  // find all model matrices
  const CMatrix<C_FLOAT64> &StoiMatrix = pModel->getStoi();
  if (StoiMatrix.array())
    {
      pObject = pModel->getObject(CCopasiObjectName("Array=Stoichiometry(ann)"));
      //std::cout << "isArray? : " << pObject->isArray() << std::endl;
      if (filter(flag, pObject))
        {
          //      pItem = new QListViewItem(matrixSubtree, "Stoichiometry(ann)");
          pItem = new Q3ListViewItem(mpModelMatrixSubtree, FROM_UTF8(pObject->getObjectName()));
          treeItems[pItem] = pObject;
        }
    }

  const CMatrix<C_FLOAT64> &RedStoiMatrix = pModel->getRedStoi();
  if (RedStoiMatrix.array())
    {
      pObject = pModel->getObject(CCopasiObjectName("Array=Reduced stoichiometry(ann)"));
      if (filter(flag, pObject))
        {
          //      pItem = new QListViewItem(matrixSubtree, "Reduced stoichiometry(ann)");
          pItem = new Q3ListViewItem(mpModelMatrixSubtree, FROM_UTF8(pObject->getObjectName()));
          treeItems[pItem] = pObject;
        }
    }

  const CMatrix<C_FLOAT64> &LinkMatrix = pModel->getL0();
  if (LinkMatrix.array())
    {
      pObject = pModel->getObject(CCopasiObjectName("Array=Link matrix(ann)"));
      if (filter(flag, pObject))
        {
          //      pItem = new QListViewItem(matrixSubtree, "Link matrix(ann)");
          pItem = new Q3ListViewItem(mpModelMatrixSubtree, FROM_UTF8(pObject->getObjectName()));
          treeItems[pItem] = pObject;
        }
    }

  removeEmptySubTree(&mpModelMatrixSubtree);

  // find all result matrices
  // Metabolic Control Analysis
  CCopasiTask *task;
  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CCopasiDataModel* pDataModel = (*CCopasiRootContainer::getDatamodelList())[0];
  assert(pDataModel != NULL);
  // MCA
  task = dynamic_cast<CCopasiTask*>((*pDataModel->getTaskList())["Metabolic Control Analysis"]);
  try
    {
      if (task && task->updateMatrices())
        {
          //for mca the result is in the method
          CMCAMethod* pMethod = dynamic_cast<CMCAMethod *>(task->getMethod());
          //           if (pMethod->getSteadyStateStatus() == CSteadyStateMethod::found)
          //             std::cout << "Steady-State exists" << std::endl;
          //           else
          //             std::cout << "NOT EXISTS" << std::endl;

          const CCopasiContainer::objectMap * pObjects = & pMethod->getObjects();
          CCopasiContainer::objectMap::const_iterator its = pObjects->begin();
          CArrayAnnotation *ann;
          for (; its != pObjects->end(); ++its)
            {
              //              std::cout << "Name = " << its->second->getObjectName() << std::endl;
              //              std::cout << "Type = " << its->second->getObjectType() << std::endl;
              ann = dynamic_cast<CArrayAnnotation*>(its->second);
              if (!ann) continue;

              //std::cout << "8 - Name : " << ann->getObjectName() << " - Type : " << ann->getObjectType() << std::endl;

              if (!ann->isEmpty() && filter(flag, ann))
                {
                  pItem = new Q3ListViewItem(this->mpResultMCASubtree, FROM_UTF8(ann->getObjectName()));
                  treeItems[pItem] = ann;
                }
            }
        }
    }
  catch (...)
  {}

  // Steady State
  task = dynamic_cast<CCopasiTask *>((*pDataModel->getTaskList())["Steady-State"]);
  try
    {
      if (task && task->updateMatrices())
        {
          //for steady state the results are in the task
          const CCopasiContainer::objectMap * pObjects = & task->getObjects();
          CCopasiContainer::objectMap::const_iterator its = pObjects->begin();
          CArrayAnnotation *ann;
          for (; its != pObjects->end(); ++its)
            {
              //              std::cout << "Name = " << its->second->getObjectName() << std::endl;
              //              std::cout << "Type = " << its->second->getObjectType() << std::endl;
              ann = dynamic_cast<CArrayAnnotation*>(its->second);
              if (!ann) continue;
              //     std::cout << "9 - Name : " << ann->getObjectName() << " - Type : " << ann->getObjectType() << std::endl;

              if (!ann->isEmpty() && filter(flag, ann))
                {
                  pItem = new Q3ListViewItem(this->mpResultSteadyStateSubtree, FROM_UTF8(ann->getObjectName()));
                  treeItems[pItem] = ann;
                }
            }
        }
    }
  catch (...)
  {}

  // Sensitivities
  task = dynamic_cast<CCopasiTask *>((*pDataModel->getTaskList())["Sensitivities"]);
  try
    {
      if (task && task->updateMatrices())
        {
          //for sensitivities the result is in the problem
          CSensProblem *sens = dynamic_cast<CSensProblem *>(task->getProblem());

          const CCopasiContainer::objectMap * pObjects = & sens->getObjects();
          CCopasiContainer::objectMap::const_iterator its = pObjects->begin();
          CArrayAnnotation *ann;
          for (; its != pObjects->end(); ++its)
            {
              //              std::cout << "Name = " << its->second->getObjectName() << std::endl;
              //              std::cout << "Type = " << its->second->getObjectType() << std::endl;
              ann = dynamic_cast<CArrayAnnotation*>(its->second);
              if (!ann) continue;
              //     std::cout << "10 - Name : " << ann->getObjectName() << " - Type : " << ann->getObjectType() << std::endl;

              //std::cout << "isArray? : " << ann->isArray() << std::endl;

              if (!ann->isEmpty() && filter(flag, ann))
                {
                  pItem = new Q3ListViewItem(this->mpResultSensitivitySubtree, FROM_UTF8(ann->getObjectName()));
                  treeItems[pItem] = (CCopasiObject *) ann;
                }
            }
        }
    }
  catch (...)
  {}

  removeEmptySubTree(&mpResultMCASubtree);
  removeEmptySubTree(&mpResultSensitivitySubtree);
  removeEmptySubTree(&mpResultSteadyStateSubtree);
  removeEmptySubTree(&mpResultMatrixSubtree);

  if (selectionMode() == Q3ListView::NoSelection)
    {
      // see if some objects are there, if yes set to single selection
      Q3ListViewItemIterator it = Q3ListViewItemIterator(this);
      while (it.current())
        {
          if (treeItems.find(it.current()) != treeItems.end())
            {
              setSelectionMode(Q3ListView::Single);
              setCurrentItem(it.current());
              it.current()->setSelected(true);
              Q3ListViewItem* parent = it.current()->parent();
              while (parent)
                {
                  parent->setOpen(true);
                  parent = parent->parent();
                }
              break;
            }
          ++it;
        }
    }
}

bool CCopasiSimpleSelectionTree::treeHasSelection()
{
  bool hasSelection = false;
  Q3ListViewItemIterator it(this);
  while (it.current())
    {
      if (it.current()->isSelected())
        {
          hasSelection = true;
          break;
        }
      ++it;
    }
  return hasSelection;
}

std::vector<const CCopasiObject * > * CCopasiSimpleSelectionTree::getTreeSelection()
{
  std::vector<const CCopasiObject * > * selection = new std::vector<const CCopasiObject * >();
  std::map< std::string, const CCopasiObject * > SelectionMap;

  if (selectionMode() == Q3ListView::Single && selectedItem())
    {
      selection->push_back(treeItems[selectedItem()]);
    }
  else
    {
      // go through the whole tree and check for selected items.
      // if the selected item has children, add all children that are leaves
      // and are connected to an object.
      // If the item is a leave and is connected to an object, add it directly
      Q3ListViewItemIterator it(this);
      Q3ListViewItem* currentItem = it.current();
      while (currentItem)
        {
          if (currentItem->isSelected())
            {
              if (currentItem->childCount() == 0)
                {
                  if (treeItems.find(currentItem) != treeItems.end())
                    SelectionMap[treeItems[currentItem]->getObjectDisplayName()] = treeItems[currentItem];
                }
              else
                {
                  Q3ListViewItemIterator it2(currentItem);
                  Q3ListViewItem* tmpItem = it2.current();
                  Q3ListViewItem* Ancestor;

                  while (tmpItem)
                    {
                      if ((tmpItem->childCount() == 0) &&
                          (treeItems.find(tmpItem) != treeItems.end()))
                        SelectionMap[treeItems[tmpItem]->getObjectDisplayName()] = treeItems[tmpItem];

                      ++it2;
                      tmpItem = it2.current();
                      if (!tmpItem) break;

                      // We continue as long as the current item is an
                      // ancestor of the tmp item, i.e., it is in the branch
                      // originating from current item.
                      for (Ancestor = tmpItem->parent();
                           Ancestor != currentItem && Ancestor;
                           Ancestor = Ancestor->parent());
                      if (!Ancestor) break;
                    }
                }
            }
          ++it;
          currentItem = it.current();
        }

      // Copy the selection set to the selection
      selection->resize(SelectionMap.size());
      std::vector< const CCopasiObject * >::iterator itSelection = selection->begin();
      std::map< std::string, const CCopasiObject * >::const_iterator itSet = SelectionMap.begin();
      std::map< std::string, const CCopasiObject * >::const_iterator endSet = SelectionMap.end();
      for (; itSet != endSet; ++itSet, ++itSelection)
        *itSelection = itSet->second;
    }
  return selection;
}

bool CCopasiSimpleSelectionTree::isMetaboliteNameUnique(const std::string & name, const CCopasiVector<CMetab> & metabolites)
{
  bool unique = true;
  bool found = false;
  unsigned int counter;
  for (counter = 0; counter < metabolites.size();++counter)
    {
      const std::string& thisName = metabolites[counter]->getObjectName();
      if (name == thisName)
        {
          if (found)
            {
              unique = false;
              break;
            }
          found = true;
        }
    }
  return unique;
}

Q3ListViewItem * CCopasiSimpleSelectionTree::findListViewItem(const CCopasiObject * object)
{
  Q3ListViewItem* item = NULL;
  std::map< Q3ListViewItem *, const CCopasiObject * >::iterator it = treeItems.begin();
  std::map< Q3ListViewItem *, const CCopasiObject * >::iterator endPos = treeItems.end();
  while (it != endPos)
    {
      if (it->second == object)
        {
          item = it->first;
          break;
        }
      ++it;
    }
  return item;
}

void CCopasiSimpleSelectionTree::selectObjects(std::vector< const CCopasiObject * > * objects)
{
  // clear selection on tree and select new objects
  clearSelection();
  unsigned int i;
  unsigned int iMax = objects->size();
  if ((selectionMode() == Q3ListView::Single || selectionMode() == Q3ListView::NoSelection) && iMax > 1)
    {
      iMax = 1;
    }
  for (i = 0; i < iMax;++i)
    {
      const CCopasiObject * object = objects->at(i);
      Q3ListViewItem* item = findListViewItem(object);
      if (!item && mpExpertSubtree)
        {
          // add the item to the expert branch
          item = new Q3ListViewItem(mpExpertSubtree,
                                    FROM_UTF8(object->getObjectDisplayName()));
          treeItems[item] = object;
        }
      // open the whole branch that contains item.
      Q3ListViewItem* parent = item->parent();
      if (parent && !(parent->isOpen()))
        {
          while (parent)
            {
              parent->setOpen(true);
              parent = parent->parent();
            }
        }
      setSelected(item, true);
    }
}

void CCopasiSimpleSelectionTree::commitClicked()
{
  if (mpOutputVector)
    {
      std::vector< const CCopasiObject * >* treeSelection = getTreeSelection();
      mpOutputVector->assign(treeSelection->begin(), treeSelection->end());
      std::vector< const CCopasiObject * >::iterator it = mpOutputVector->begin();
      while (it != mpOutputVector->end())
        {
          if (*it == NULL)
            {
              it = mpOutputVector->erase(it);
            }
          else
            {
              ++it;
            }
        }
      delete treeSelection;
    }
}

void CCopasiSimpleSelectionTree::setOutputVector(std::vector< const CCopasiObject * > * outputVector)
{
  mpOutputVector = outputVector;
  if (mpOutputVector)
    {
      selectObjects(mpOutputVector);
    }
}

// static
bool CCopasiSimpleSelectionTree::filter(const SelectionFlag & flag, const CCopasiObject * pObject)
{
  if (pObject == NULL)
    return false;

  if (flag == NO_RESTRICTION)
    return true;

  // if pObject is an array than the flag must be one based on array also.
  if (pObject->isArray())
    {
      /*
       std::cout << "on filter" << std::endl;
       std::cout << "object cn = " << pObject->getCN() << std::endl;
       std::cout << "object: " << pObject->getObjectType() << " - " << pObject->getObjectName() << std::endl;

          std::cout << "flag: " << flag << std::endl;
      */
      if (flag & BASE_ARRAY)
        return true;
    }

  // Check whether the value is of the desired numeric type.
  if (flag & NUMERIC)
    {
      if (!pObject->isValueDbl() && !pObject->isValueInt())
        return false;
    }
  else if ((flag & BASE_DOUBLE) && !pObject->isValueDbl())
    return false;
  else if ((flag & BASE_INTEGER) && !pObject->isValueInt())
    return false;

  if (pObject->isReference())
    {
      // CModelEntity needs to be check more thoroughly
      const CModelEntity * pEntity =
        dynamic_cast< const CModelEntity * >(pObject->getObjectParent());

      if (pEntity)
        {
          // CModelEntity::ASSIGNMENT may have no initial value or rate
          if (pEntity->getStatus() == CModelEntity::ASSIGNMENT &&
              (pObject->getObjectName().compare(0, 7, "Initial") == 0 ||
               pObject->getObjectName().find("Rate") != std::string::npos))
            return false;

          // CModelEntity::FIXED may have no rate
          if (pEntity->getStatus() == CModelEntity::FIXED &&
              pObject->getObjectName().find("Rate") != std::string::npos)
            return false;

          // CModelEntity are handled differently dependent on the type
          // of EXPRESSION.
          if (flag & BASE_EXPRESSION)
            {
              // TRANSIENT_EXPRESSION
              if (flag & BASE_TRANSIENT)
                {
                  if (pEntity->getStatus() == CModelEntity::FIXED &&
                      pObject->getObjectName().compare(0, 7, "Initial") == 0)
                    return false;
                }

              // INITIAL_EXPRESSION
              else if (flag & BASE_INITIAL)
                {
                  if (pEntity->getStatus() != CModelEntity::ASSIGNMENT &&
                      (pObject->getObjectName().compare(0, 7, "Initial") != 0 &&
                       pObject->getObjectName() != "Quantity Conversion Factor"))
                    return false;
                }

              // Everything else is allowed in EXPRESSION
              return true;
            }

          // TARGET_EVENT -> only Transient Volumes, Transient Concentrations, and Transient Values are allowed
          if ((flag & BASE_TRANSIENT) && (flag & BASE_MODEL))
            {
              if ((pObject->getObjectName().find("Time") != std::string::npos) ||
                  (pObject->getObjectName().find("Initial") != std::string::npos) ||
                  // (pObject->getObjectName().find("ParticleNumber") != std::string::npos) ||
                  (pObject->getObjectName().find("Rate") != std::string::npos))
                return false;
            }

          // PARAMETER -> Initial Volumes, Initial Concentrations, Initial Particle Number,
          // Initial Values, and Reaction Parameters are allowed
          if ((flag & BASE_INITIAL) && (flag & BASE_TASK))
            {
              if (pObject->getObjectName().find("Time") != std::string::npos)
                return false;

              if (pObject->getObjectName().find("Initial") == std::string::npos)
                return false;
            }

          // INITIAL_VALUE
          if ((flag & BASE_INITIAL)
              && pObject->getObjectName().compare(0, 7, "Initial") != 0)
            return false;

          // TRANSIENT_VALUE
          if ((flag & BASE_TRANSIENT) &&
              pObject->getObjectName().compare(0, 7, "Initial") == 0)
            return false;

          // This CModelEntity is valid.
          return true;
        }

      // ** Reaction **
      // CReaction needs to be check more thoroughly
      const CReaction * pReaction =
        dynamic_cast< const CReaction * >(pObject->getObjectParent());

      if (pReaction)
        {
          // These are transient values which may be used in expressions.
          if ((flag & BASE_INITIAL) &&
              !(flag & BASE_EXPRESSION))
            return false;

          // TARGET_EVENT -> only Transient Volumes, Transient Concentrations, and Transient Values are allowed
          /* as it conflicts with TRANSIENT_EXPRESSION and it is not yet used, we temporarily ignore the following (04.12.2008).
                if ((flag & BASE_TRANSIENT) && (flag & BASE_MODEL))
                  {
                    if (pObject->getObjectName().find("Flux") != std::string::npos)
                      return false;
                  }
          */

          // Every other value of CReaction is valid.
          return true;
        }

      // Reaction Parameters
      pReaction =
        dynamic_cast< const CReaction * >(pObject->getObjectAncestor("Reaction"));

      if (pReaction)
        {
          // Local reaction parameters may not be used in any expression in the model.
          //          if ((flag & BASE_MODEL))
          if ((flag & BASE_GLOBAL))
            return false;

          // These are initial values which may be used in expressions.
          if (!(flag & BASE_EXPRESSION) &&
              (flag & BASE_TRANSIENT))
            return false;

          return true;
        }
    }
  // CCopasiTimer may not be used in the model.
  else if (dynamic_cast< const CCopasiTimer * >(pObject) &&
           (flag & BASE_MODEL))
    return false;

  // All tests passed :)
  return true;
}

/*
 * remove all empty subtree
 */
void CCopasiSimpleSelectionTree::removeEmptySubTree(Q3ListViewItem ** ppSubTree)
{
  if ((*ppSubTree)->childCount() == 0)
    pdelete(*ppSubTree);
}
