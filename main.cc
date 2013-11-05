// Lecture d'un fichier Rtstruct et conversion en vtkImageData, sortie en VTI
// \author Olivier Saut
#include <vector>
#include <map>

#include <vtkVersion.h>
// * Lecture du RTStruct
#include <vtkCellData.h>
#include <vtkGDCMPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkAppendPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkImageGaussianSmooth.h>

// Ecriture en ImageData
#include <vtkPointData.h>
#include <vtkImageData.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>


std::string getPrefix(std::string str) {
    std::string relName=str.substr(str.find_last_of("/")+1);
    std::string prefix=relName.substr(0,relName.find_last_of("."));
    return prefix;
}



void stripSpace(std::string &str) {
    std::string resStr="";
        for (int i=0;i<str.length();++i) {
                if (str[i]!=' ')
                    resStr+=str[i];
                else
                    resStr+='_';
        }
    str=resStr;
}


void save_VTP_as_image(std::string name, vtkSmartPointer<vtkPolyData> polygon, std::string outDir) {
    vtkSmartPointer<vtkLinearExtrusionFilter> extrude=vtkSmartPointer<vtkLinearExtrusionFilter>::New();
#if VTK_MAJOR_VERSION >= 6
        extrude->SetInputData(polygon);
#else
        extrude->SetInput(polygon);
#endif
    extrude->SetScaleFactor(12);
    extrude->SetExtrusionTypeToNormalExtrusion();
    extrude->SetVector(0,0,1);

    // ***
    // * Préparation de l'image
    // ***
    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    double bounds[6]; polygon->GetBounds(bounds);
    double spacing[3] = {2, 2, 2};
    image->SetSpacing(spacing);

    int dim[3];
    for (int i = 0; i < 3; i++)
    {
        dim[i] = static_cast<int>(ceil(1.2*(bounds[i * 2 + 1] - bounds[i * 2]) / spacing[i]));
    }

    // dim[2]=1; Pour sélectionner une tranche
    std::cout << "\t\tDimensions = " <<  dim[0] << "x" << dim[1] << "x" << dim[2] << std::endl;
    image->SetDimensions(dim);
    image->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);

    double origin[3];
    // offset ?
    origin[0] = bounds[0] -0.1*(bounds[1]-bounds[0]); // spacing[0] / 2;
    origin[1] = bounds[2] -0.1*(bounds[3]-bounds[2]);//+ spacing[1] / 2;
    origin[2] = bounds[4] -0.1*(bounds[5]-bounds[4]);//+ spacing[2] / 2;
    std::cout <<  "\t\tOrigine = (" << origin[0] << ", " << origin[1] << ", " << origin[2] << ")" << std::endl;
    image->SetOrigin(origin);
#if VTK_MAJOR_VERSION >= 6
    image->AllocateScalars(VTK_FLOAT,1);
#else
    image->SetScalarTypeToFloat();
    image->AllocateScalars();
#endif
    // fill the image with foreground voxels:
    float inval = 255.0;
    float outval = 0;
    vtkIdType count = image->GetNumberOfPoints();
    for (vtkIdType i = 0; i < count; ++i)
    {
        image->GetPointData()->GetScalars()->SetTuple1(i, inval);
    }

    // ***
    // * Conversion
    // ***
    // On clippe l'image avec le polydata
    vtkSmartPointer<vtkPolyDataToImageStencil> conv=vtkSmartPointer<vtkPolyDataToImageStencil>::New();
#if VTK_MAJOR_VERSION >= 6
        conv->SetInputData(extrude->GetOutput());
#else
        conv->SetInput(extrude->GetOutput());
#endif

    conv->SetOutputOrigin(origin);
    conv->SetOutputSpacing(spacing);
    conv->SetTolerance(1e-6);
    conv->SetOutputWholeExtent(image->GetExtent());
    conv->Update();

    vtkSmartPointer<vtkImageStencil> stenc=vtkSmartPointer<vtkImageStencil>::New();
#if VTK_MAJOR_VERSION >= 6
        stenc->SetInputData(image);
        stenc->SetStencilData(conv->GetOutput());
#else
        stenc->SetInput(image);
        stenc->SetStencil(conv->GetOutput());
#endif
    stenc->ReverseStencilOff();
    stenc->SetBackgroundValue(outval);
    stenc->Update();

    // ***
    // * Ecriture sur le disque
    // ***

     // Lissage pour un joli résultat
    vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter = vtkSmartPointer<vtkImageGaussianSmooth>::New();
    gaussianSmoothFilter->SetInputConnection(stenc->GetOutputPort());
    gaussianSmoothFilter->SetStandardDeviation(4.0);
    gaussianSmoothFilter->Update(); gaussianSmoothFilter->UpdateWholeExtent();

    // Ecriture en tant qu'image
    vtkSmartPointer<vtkXMLImageDataWriter> ww=vtkSmartPointer<vtkXMLImageDataWriter>::New();
    ww->SetFileName(std::string(outDir+"/"+name+".vti").c_str());
#if VTK_MAJOR_VERSION >= 6
        ww->SetInputData(gaussianSmoothFilter->GetOutput());
#else
        ww->SetInput(gaussianSmoothFilter->GetOutput());
#endif
    ww->Write();

}


// \brief Prend en argument le nom du fichier RTStruct
int main(int argc, char *argv[])
{
    if( argc < 2 )
    {
        std::cerr << argv[0] << " input.dcm (outdir)\n";
        return 1;
    }

    // ***
    // * Lecture de la structure
    // ***
    const char * filename = argv[1];
    vtkSmartPointer<vtkGDCMPolyDataReader> reader = vtkSmartPointer<vtkGDCMPolyDataReader>::New();
    reader->SetFileName( filename );
    reader->Update();

    std::string outDir = (argc>2) ? argv[2] : ".";

    vtkSmartPointer<vtkAppendPolyData> append = vtkSmartPointer<vtkAppendPolyData>::New();

    vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    char fname[300]; std::string prefix=getPrefix(filename);
    int n = reader->GetNumberOfOutputPorts();
    std::map<std::string, vtkSmartPointer<vtkPolyData> > listePolys;
    for(int i = 0; i < n; ++i) {
        std::string port_name="";
        const int num_arrays_cell=reader->GetOutput(i)->GetCellData()->GetNumberOfArrays();
        if(num_arrays_cell>0)
            port_name= std::string(reader->GetOutput(i)->GetCellData()->GetArrayName(0));
        const int num_arrays_point=reader->GetOutput(i)->GetPointData()->GetNumberOfArrays();
        if(num_arrays_point>0)
            port_name= std::string(reader->GetOutput(i)->GetPointData()->GetArrayName(0));
        stripSpace(port_name);
        snprintf(fname, 300, "%s/%d-%s.vtp", outDir.data(), i, port_name.c_str());

        writer->SetFileName(fname);
#if VTK_MAJOR_VERSION >= 6
        writer->SetInputData(reader->GetOutput(i));
#else
        writer->SetInput(reader->GetOutput(i));
#endif
        listePolys.insert(std::pair<std::string, vtkSmartPointer<vtkPolyData> > (port_name, reader->GetOutput(i)));
        writer->Write();

#if VTK_MAJOR_VERSION >= 6
        append->AddInputData( reader->GetOutput(i) );
#else
        append->AddInput( reader->GetOutput(i) );
#endif
    }

    std::map<std::string, vtkSmartPointer<vtkPolyData> >::const_iterator it;
    for(it=listePolys.begin(); it!= listePolys.end();++it) {
        if(it->first != "") {
            std::cout << "\tSaving " << it->first << "..." << std::endl;
            save_VTP_as_image(it->first, it->second, outDir);
        }
    }

    return 0;
}
