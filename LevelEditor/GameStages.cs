using System;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;

namespace LevelEditor
{
    public class GameStages : List<String>
    {

        private const string STAGES_FILE = "/res/data/stages.dat";

        public static GameStages Load()
        {
            GameStages result = new GameStages();
            string path = Path.GetDirectoryName(Application.ExecutablePath);

            try
            {
                BinaryReader reader = new BinaryReader(new FileStream(path + STAGES_FILE, FileMode.Open));

                int numStages = reader.ReadInt32();

                for (int i = 0; i < numStages; i++)
                {
                    int strLen = reader.ReadInt32();
                    result.Add(new string(reader.ReadChars(strLen)));
                }

                reader.Close();
            }
            catch (IOException)
            {
                MessageBox.Show("Stages file not found", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            return result;
        }


        public static void Store(GameStages gameStages)
        {
            string path = Path.GetDirectoryName(Application.ExecutablePath);

            Directory.CreateDirectory(path + "/res/data");

            try
            {
                BinaryWriter writer = new BinaryWriter(new FileStream(path + STAGES_FILE, FileMode.Create));

                writer.Write(gameStages.Count);

                foreach (String file in gameStages)
                {
                    writer.Write(file.Length);
                    writer.Write(file.ToCharArray());
                }

                writer.Close();
            }
            catch (IOException)
            {
                MessageBox.Show("An error occurred while saving data", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}
