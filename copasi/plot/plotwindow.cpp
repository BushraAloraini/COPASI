/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/plot/Attic/plotwindow.cpp,v $
   $Revision: 1.11 $
   $Name:  $
   $Author: ssahle $ 
   $Date: 2004/08/05 12:54:16 $
   End CVS Header */

// the window containing the plot and buttons for supported operations

#include <qtoolbar.h>
#include <qprinter.h>

#include "plotwindow.h" 
//#include "plotspec.h"
#include "CopasiPlot.h" 
//#include "CPlotSpec2Vector.h"

// #include "plotwidget1.h"

// taken from qwt examples/bode
class PrintFilter: public QwtPlotPrintFilter
  {
  public:
    PrintFilter() {};

    virtual QFont font(const QFont &f, Item, int) const
      {
        QFont f2 = f;
        f2.setPointSize(f.pointSize() * 0.75);
        return f2;
      }
  };

//-----------------------------------------------------------------------------

PlotWindow::PlotWindow(CPlotSpec2Vector* psv, const CPlotSpecification* ptrSpec)
{
  // set up the GUI - the toolbar
  QToolBar * plotTools = new QToolBar(this, "plot operations");
  plotTools->setLabel("Plot Operations");

  zoomButton = new QToolButton(plotTools, "zoom");
  zoomButton->setText("Zoom");
  zoomButton->setTextLabel("Zoom");
  //zoomButton->setToggleButton(true);

  QToolButton * printButton = new QToolButton(plotTools, "print plot");
  printButton -> setTextLabel("Print plot");
  printButton -> setText("Print");

  plotTools->setStretchableWidget(new QWidget(plotTools));

  plot = new CopasiPlot(psv, ptrSpec, this);
  setCentralWidget(plot);

  connect(zoomButton, SIGNAL(clicked()), this, SLOT(enableZoom()));
  connect(printButton, SIGNAL(clicked()), this, SLOT(printPlot()));
  connect(plot, SIGNAL(plotMouseReleased(const QMouseEvent &)), this, SLOT(mouseReleased(const QMouseEvent&)));
}

bool PlotWindow::initFromSpec(CPlotSpec2Vector* psv, const CPlotSpecification* ptrSpec)
{
  return plot->initFromSpec(psv, ptrSpec);
}

//-----------------------------------------------------------------------------

void PlotWindow::enableZoom()
{
  zoomButton->setEnabled(false);
  plot->enableZoom(true);
}

//-----------------------------------------------------------------------------

void PlotWindow::mouseReleased(const QMouseEvent &e)
{
  //TODO: if midbutton is clicked and we're zoomed out completely, zoomButton need to be enabled as well

  if (e.button() == RightButton)
    zoomButton->setEnabled(true);
}

//-----------------------------------------------------------------------------

void PlotWindow::printPlot()
{
  QPrinter printer;

  QString docName = plot->title();
  if (docName.isEmpty())
    {
      //docName.replace (QRegExp (QString::fromLatin1 ("\n")), tr (" -- "));
      docName = QString::fromLatin1("A plot of selected Copasi output");
      printer.setDocName (docName);
    }

  printer.setCreator(docName);
  printer.setOrientation(QPrinter::Landscape);

  if (printer.setup())
    plot->print(printer, PrintFilter());
}

//-----------------------------------------------------------------------------

PlotWindow::~PlotWindow()
{}

void PlotWindow::takeData(const std::vector<C_FLOAT64> & data)
{if (plot) plot->takeData(data);};

void PlotWindow::updatePlot()
{if (plot) plot->updatePlot();};
