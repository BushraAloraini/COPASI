/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/StateSubwidget.ui.h,v $
   $Revision: 1.20 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2006/03/30 19:05:29 $
   End CVS Header */

/****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you want to add, delete, or rename functions or slots, use
 ** Qt Designer to update this file, preserving your code.
 **
 ** You should not define a constructor or destructor in this file.
 ** Instead, write your code in functions called init() and destroy().
 ** These will automatically be called by the form's constructor and
 ** destructor.
 *****************************************************************************/

#include <sstream>

#include "copasi.h"

#include "qtUtilities.h"
#include "listviews.h"

#include "model/CChemEqInterface.h"
#include "model/CModel.h"
#include "model/CMetabNameInterface.h"
#include "steadystate/CSteadyStateTask.h"
#include "steadystate/CSteadyStateProblem.h"
#include "steadystate/CEigen.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "optimization/COptProblem.h"
#include "optimization/COptTask.h"

void StateSubwidget::init()
{
  topLabel->setText("");
  displayOptimizationTab(false);
  //bla
}

void StateSubwidget::displayOptimizationTab(bool displayOptTab)
{
  if (displayOptTab)
    {
      tabWidget->insertTab(TabPage, "OptimizationResults", 0);
      tabWidget->setCurrentPage(0);
    }
  else
    tabWidget->removePage(TabPage);
}

bool StateSubwidget::loadMetabolites(const CModel* model)
{
  const CCopasiVector<CMetab>& metabs = model->getMetabolites();
  C_INT32 i, imax = metabs.size();

  concentrationsTable->setNumRows(imax);
  numbersTable->setNumRows(imax);

  QString name;
  for (i = 0; i < imax; ++i)
    {
      name = FROM_UTF8(CMetabNameInterface::getDisplayName(model, *metabs[i]));

      concentrationsTable->setText(i, 0, name);
      concentrationsTable->setText(i, 1, QString::number(metabs[i]->getConcentration()));
      concentrationsTable->setText(i, 2, QString::number(metabs[i]->getConcentrationRate()));
      concentrationsTable->setText(i, 3, QString::number(metabs[i]->getTransitionTime()));

      numbersTable->setText(i, 0, name);
      numbersTable->setText(i, 1, QString::number(metabs[i]->getValue()));
      numbersTable->setText(i, 2, QString::number(metabs[i]->getRate()));
      numbersTable->setText(i, 3, QString::number(metabs[i]->getTransitionTime()));
    }

  return true;
}

bool StateSubwidget::loadReactions(const CModel * model)
{
  const CCopasiVector<CReaction>& reacs = model->getReactions();
  C_INT32 i, imax = reacs.size();

  tableFlux->setNumRows(imax);

  for (i = 0; i < imax; ++i)
    {
      tableFlux->setText(i, 0, FROM_UTF8(reacs[i]->getObjectName()));
      tableFlux->setText(i, 1, QString::number(reacs[i]->getFlux()));
      tableFlux->setText(i, 2, QString::number(reacs[i]->getParticleFlux()));
      tableFlux->setText(i, 3, FROM_UTF8(CChemEqInterface::getChemEqString(model, *reacs[i], false)));
    }
  return true;
}

void StateSubwidget::loadJacobian(const CSteadyStateTask * task)
{
  const CMatrix< C_FLOAT64 > & jacobian = task->getJacobian();

  tableJacobian->setNumRows(jacobian.numRows());
  tableJacobian->setNumCols(jacobian.numCols());

  C_INT32 i, imax = jacobian.numRows();
  C_INT32 j, jmax = jacobian.numCols();
  for (i = 0; i < imax; ++i)
    for (j = 0; j < jmax; ++j)
      {
        tableJacobian->setText(i, j, QString::number(jacobian(i, j)));
      }

  QString name;
  const CCopasiVector<CMetab>& metabs = task->getProblem()->getModel()->getMetabolites();
  for (i = 0; i < imax; ++i)
    {
      name = FROM_UTF8(CMetabNameInterface::getDisplayName(task->getProblem()->getModel(), *metabs[i]));
      tableJacobian->horizontalHeader()->setLabel(i, name);
      tableJacobian->verticalHeader()->setLabel(i, name);
    }

  //Eigenvalues...
  const CVector< C_FLOAT64 > & eigen_i = task->getEigenValues().getI();
  const CVector< C_FLOAT64 > & eigen_r = task->getEigenValues().getR();

  imax = eigen_i.size();
  tableEigenValues->setNumRows(imax);
  for (i = 0; i < imax; ++i)
    {
      tableEigenValues->setText(i, 0, QString::number(eigen_r[i]));
      tableEigenValues->setText(i, 1, QString::number(eigen_i[i]));
    }

  //JacobianX
  const CMatrix< C_FLOAT64 > & jacobianX = task->getJacobianReduced();

  tableJacobianX->setNumRows(jacobianX.numRows());
  tableJacobianX->setNumCols(jacobianX.numCols());

  imax = jacobianX.numRows();
  jmax = jacobianX.numCols();
  for (i = 0; i < imax; ++i)
    for (j = 0; j < jmax; ++j)
      {
        tableJacobianX->setText(i, j, QString::number(jacobianX(i, j)));
      }

  const CCopasiVector<CMetab>& metabsX = task->getProblem()->getModel()->getMetabolitesInd();
  for (i = 0; i < imax; ++i)
    {
      name = FROM_UTF8(CMetabNameInterface::getDisplayName(task->getProblem()->getModel(), *metabsX[i]));
      tableJacobianX->horizontalHeader()->setLabel(i, name);
      tableJacobianX->verticalHeader()->setLabel(i, name);
    }

  //Eigenvalues...
  const CVector< C_FLOAT64 > & eigen_iX = task->getEigenValuesReduced().getI();
  const CVector< C_FLOAT64 > & eigen_rX = task->getEigenValuesReduced().getR();

  imax = eigen_iX.size();
  tableEigenValuesX->setNumRows(imax);
  for (i = 0; i < imax; ++i)
    {
      tableEigenValuesX->setText(i, 0, QString::number(eigen_rX[i]));
      tableEigenValuesX->setText(i, 1, QString::number(eigen_iX[i]));
    }

  //stability report
  stabilityTextEdit->setReadOnly(true);

  std::ostringstream ss;
  ss << task->getEigenValuesReduced();

  stabilityTextEdit->setText(FROM_UTF8(ss.str()));
  return;
}

void StateSubwidget::showUnits()
{
  //CCopasiDataModel::Global->getModel()->getVolumeUnit();

  concentrationsTable->horizontalHeader()->setLabel(concentrationsTable->numCols() - 1, tr("Transition Time\n("\
      + FROM_UTF8(CCopasiDataModel::Global->getModel()->getTimeUnitName()) + ")"));

  concentrationsTable->horizontalHeader()->setLabel(concentrationsTable->numCols() - 2, tr("Rate\n(per "\
      + FROM_UTF8(CCopasiDataModel::Global->getModel()->getTimeUnitName()) + ")"));

  concentrationsTable->horizontalHeader()->setLabel(concentrationsTable->numCols() - 3, tr("Concentration\n("\
      + FROM_UTF8(CCopasiDataModel::Global->getModel()->getConcentrationUnitName()) + ")"));

  numbersTable->horizontalHeader()->setLabel(numbersTable->numCols() - 1, tr("Transition Time\n("\
      + FROM_UTF8(CCopasiDataModel::Global->getModel()->getTimeUnitName()) + ")"));
  numbersTable->horizontalHeader()->setLabel(numbersTable->numCols() - 2, tr("Rate\n(per "\
      + FROM_UTF8(CCopasiDataModel::Global->getModel()->getTimeUnitName()) + ")"));

  tableFlux->horizontalHeader()->setLabel(numbersTable->numCols() - 3, tr("Flux\n("\
                                          + FROM_UTF8(CCopasiDataModel::Global->getModel()->getQuantityRateUnitName()) + ")"));

  tableFlux->horizontalHeader()->setLabel(numbersTable->numCols() - 2, tr("Particle flux\n(particle/"\
                                          + FROM_UTF8(CCopasiDataModel::Global->getModel()->getTimeUnitName()) + ")"));
}

bool StateSubwidget::loadAll(const CSteadyStateTask * task)
{
  CState * pState = const_cast<CState *>(task->getState());
  CModel * pModel = task->getProblem()->getModel();

  pModel->setState(*pState);
  pModel->applyAssignments();
  pModel->updateRates();

  // editing units here

  if (task->getResult() == CSteadyStateMethod::found)
    topLabel->setText("A steady state with given resolution was found.");
  else if (task->getResult() == CSteadyStateMethod::notFound)
    topLabel->setText("No steady state with given resolution was found!");
  else if (task->getResult() == CSteadyStateMethod::foundEquilibrium)
    topLabel->setText("An equilibrium steady state (zero fluxes) was found.");
  else if (task->getResult() == CSteadyStateMethod::foundNegative)
    topLabel->setText("An invalid steady state (negative concentrations) was found.");

  else
    topLabel->setText("A steady state with given resolution couldn't be found.");

  if (!loadMetabolites(pModel)) return false;
  if (!loadReactions(pModel)) return false;

  const CSteadyStateProblem* pProblem =
    dynamic_cast<const CSteadyStateProblem *>(task->getProblem());
  assert(pProblem);

  if (pProblem->isJacobianRequested() ||
      pProblem->isStabilityAnalysisRequested())
    {
      tabWidget->setTabEnabled(tabWidget->page(3), true);
      tabWidget->setTabEnabled(tabWidget->page(4), true);
      tabWidget->setTabEnabled(tabWidget->page(5), true);
      loadJacobian(task);
    }

  else
    {
      tabWidget->setTabEnabled(tabWidget->page(3), false);
      tabWidget->setTabEnabled(tabWidget->page(4), false);
      tabWidget->setTabEnabled(tabWidget->page(5), false);
    }

  return true;
}

bool StateSubwidget::clear()
{
  topLabel->setText("No result available, please execute the steady-state task.");

  concentrationsTable->setNumRows(0);
  numbersTable->setNumRows(0);

  tableFlux->setNumRows(0);

  tableJacobian->setNumRows(0);
  tableJacobian->setNumCols(0);

  tableEigenValues->setNumRows(0);

  tableJacobianX->setNumRows(0);
  tableJacobianX->setNumCols(0);

  tableEigenValuesX->setNumRows(0);

  stabilityTextEdit->setText("");

  return true;
}
