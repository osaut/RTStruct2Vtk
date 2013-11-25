require 'dicom'
require 'date'
include DICOM

def get_date fname
    dcm=DObject.read(fname)
    Date.parse(dcm.value("0008,0020"))
end

exename = File.absolute_path("./rtsr.out")
fail unless File.exists?(exename)

Dir.chdir(ARGV[0])
patients=Dir.glob("patient*")
patients.each { |patient|
    puts "Processing #{patient}..."
        Dir.mkdir("#{patient}/volumes") unless Dir.exists?("#{patient}/volumes")
        exams=Dir.glob("#{patient}/p_*")
        exams.each { |exam|
            contours=Dir.glob("#{exam}/RTSTRUCT.*.dcm")
            dates=contours.map { |f| get_date(f)}.uniq

            contours.each { |cc|

                dcm_file=File.absolute_path(cc)
                if (contours.size==dates.size)
                    date=get_date(dcm_file)
                    dirname=File.join(ARGV[0], patient,"volumes", date.strftime('%Y_%m_%d'))
                else
                    dirname=File.join(ARGV[0], patient,"volumes", File.basename(cc,".dcm").sub(/RTSTRUCT./, ''))
                end
                Dir.mkdir(dirname) unless Dir.exists?(dirname)
                system  "#{exename} #{dcm_file} #{dirname}"
            }
        }
}
