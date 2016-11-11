import java.io.*;

public class TestReadFile{

	public static void main(String args[]) {

    try
        {
		DataInputStream dataIn = new DataInputStream(new FileInputStream("C:\\Users\\vladi\\Documents\\AI16\\cpp-cgdk\\dump.txt"));
            
            //drawLine(self.getX(), self.getY(), self.getX() + 100.0 * (world.getTickIndex() / 20000.0), self.getY() + 100.0 * (world.getTickIndex() / 20000.0));
            
            for (int i = 0; i <= 100; i++)
            {
                int tick = dataIn.readInt();
                int count = dataIn.readInt();
                
                System.out.println(tick);
                System.out.println(count);
                
                for (int j = 0; j < count; j++)
                {
                    String s = dataIn.readLine();
                    Double x = dataIn.readDouble();
                    Double y = dataIn.readDouble();
                    
                    //if (tick != world.getTickIndex())
                    //    continue;
                    
                    //drawLine(self.getX(), self.getY(), self.getX() + x, self.getY() + y);
                }
            }
            dataIn.close();
             }
        catch (IOException ex)
        {
            //graphics.setColor(Color.RED);
            // drawLine(self.getX(), self.getY(), self.getX() + 100.0 * (world.getTickIndex() / 20000.0), self.getY() + 100.0 * (world.getTickIndex() / 20000.0));
            System.out.println("NO!!!!!");
        }

	}

}


